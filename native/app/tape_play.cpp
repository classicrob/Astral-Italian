#include "tape_play.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <atomic>
#include <chrono>
#include <ctime>
#include <mutex>
#include <sstream>
#include <thread>

#include "headless/platform.hpp"  // decodeSample

namespace PetariNative::App::TapePlay {
namespace H = PetariNative::Headless;
namespace {

constexpr double kPositionLimit = 0.01;       // the same limit as tools/port/replay_tape.py
constexpr std::uint64_t kWaitSeamLimit = 36000;  // ~10 minutes of loops waiting for a segment's play-ready seam

// --- a small JSON reader for the sidecar (objects, arrays, strings, numbers, true/false/null) ---
struct Json {
    enum class Type { Null, Bool, Number, String, Array, Object } type = Type::Null;
    bool boolean = false;
    double number = 0;
    std::string text;
    std::vector<Json> items;
    std::vector<std::pair<std::string, Json>> members;
    const Json* find(const char* key) const {
        for (const auto& m : members) if (m.first == key) return &m.second;
        return nullptr;
    }
};
class JsonReader {
public:
    explicit JsonReader(const std::string& s) : mS(s) {}
    Json parse() {
        auto value = parseValue();
        skip();
        if (mAt != mS.size()) fail("trailing text");
        return value;
    }
private:
    [[noreturn]] void fail(const char* why) const { throw H::ReplayError(std::string("bad tape sidecar: ") + why); }
    void skip() { while (mAt < mS.size() && std::isspace(static_cast<unsigned char>(mS[mAt]))) ++mAt; }
    char peek() { skip(); if (mAt >= mS.size()) fail("unexpected end"); return mS[mAt]; }
    void expect(char c) { if (peek() != c) fail("unexpected character"); ++mAt; }
    std::string parseString() {
        expect('"');
        std::string out;
        while (mAt < mS.size() && mS[mAt] != '"') {
            char c = mS[mAt++];
            if (c == '\\') {
                if (mAt >= mS.size()) fail("bad escape");
                const char e = mS[mAt++];
                out += e == 'n' ? '\n' : e == 't' ? '\t' : e == 'r' ? '\r' : e;  // \" \\ \/ and the rest as themselves
            } else {
                out += c;
            }
        }
        if (mAt >= mS.size()) fail("unterminated string");
        ++mAt;
        return out;
    }
    Json parseValue() {
        Json v;
        const char c = peek();
        if (c == '{') {
            ++mAt;
            v.type = Json::Type::Object;
            if (peek() == '}') { ++mAt; return v; }
            for (;;) {
                auto key = parseString();
                expect(':');
                v.members.emplace_back(std::move(key), parseValue());
                if (peek() == ',') { ++mAt; continue; }
                expect('}');
                return v;
            }
        }
        if (c == '[') {
            ++mAt;
            v.type = Json::Type::Array;
            if (peek() == ']') { ++mAt; return v; }
            for (;;) {
                v.items.push_back(parseValue());
                if (peek() == ',') { ++mAt; continue; }
                expect(']');
                return v;
            }
        }
        if (c == '"') { v.type = Json::Type::String; v.text = parseString(); return v; }
        if (mS.compare(mAt, 4, "true") == 0) { mAt += 4; v.type = Json::Type::Bool; v.boolean = true; return v; }
        if (mS.compare(mAt, 5, "false") == 0) { mAt += 5; v.type = Json::Type::Bool; return v; }
        if (mS.compare(mAt, 4, "null") == 0) { mAt += 4; return v; }
        char* end = nullptr;
        v.number = std::strtod(mS.c_str() + mAt, &end);
        if (end == mS.c_str() + mAt) fail("unexpected token");
        mAt = static_cast<std::size_t>(end - mS.c_str());
        v.type = Json::Type::Number;
        return v;
    }
    const std::string& mS;
    std::size_t mAt = 0;
};

std::string slurp(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw H::ReplayError("cannot open " + path.string());
    std::ostringstream out;
    out << in.rdbuf();
    return out.str();
}

std::vector<std::string> splitCsv(const std::string& line) {
    std::vector<std::string> out;
    std::string cur;
    bool quoted = false;
    for (char c : line) {
        if (c == '"') quoted = !quoted;
        else if (c == ',' && !quoted) { out.push_back(std::move(cur)); cur.clear(); }
        else cur += c;
    }
    out.push_back(std::move(cur));
    return out;
}

std::string jsonString(const std::string& s) {
    std::string out = "\"";
    for (char c : s) {
        if (c == '"' || c == '\\') out += '\\';
        out += c;
    }
    return out + "\"";
}

}  // namespace

Meta parseSidecar(const std::string& text) {
    const Json root = JsonReader(text).parse();
    if (root.type != Json::Type::Object) throw H::ReplayError("bad tape sidecar: not an object");
    Meta meta;
    const auto number = [&](const Json& obj, const char* key) {
        const Json* v = obj.find(key);
        if (!v || v->type != Json::Type::Number || v->number < 0) throw H::ReplayError(std::string("bad tape sidecar: missing ") + key);
        return static_cast<std::uint64_t>(v->number);
    };
    meta.seams = number(root, "seams");
    if (const Json* v = root.find("complete")) meta.complete = v->boolean;
    if (const Json* v = root.find("exe_sha256")) meta.exeSha256 = v->text;
    if (const Json* v = root.find("mods"))
        for (const auto& m : v->items) meta.mods.push_back(m.text);
    const Json* segments = root.find("segments");
    if (segments)
        for (const auto& g : segments->items) {
            SegmentMeta seg;
            seg.startRow = number(g, "start_row");
            seg.endRow = number(g, "end_row");
            const Json* stage = g.find("stage");
            seg.stage = stage ? stage->text : std::string();
            seg.scenario = static_cast<int>(number(g, "scenario"));
            seg.seed = static_cast<std::uint32_t>(number(g, "rng_seed"));
            meta.segments.push_back(std::move(seg));
        }
    if (meta.segments.empty()) {  // tapes written before segments: the first play-ready seam to the end
        if (const Json* ps = root.find("play_ready_seams"); ps && !ps->items.empty()) {
            const auto& r = ps->items.front();
            SegmentMeta seg;
            seg.startRow = number(r, "row");
            seg.endRow = meta.seams ? meta.seams - 1 : 0;
            seg.stage = r.find("stage") ? r.find("stage")->text : std::string();
            seg.scenario = static_cast<int>(number(r, "scenario"));
            seg.seed = static_cast<std::uint32_t>(number(r, "rng_seed"));
            meta.segments.push_back(std::move(seg));
        }
    }
    if (const Json* starts = root.find("ready_seams"))
        for (const auto& r : starts->items) {
            SegmentMeta seg;
            seg.startRow = number(r, "row");
            seg.stage = r.find("stage") ? r.find("stage")->text : std::string();
            seg.scenario = static_cast<int>(number(r, "scenario"));
            seg.seed = static_cast<std::uint32_t>(number(r, "rng_seed"));
            meta.sceneStarts.push_back(std::move(seg));
        }
    return meta;
}

Session::Session(const std::filesystem::path& dir, Mode mode) : mMode(mode) {
    mMeta = parseSidecar(slurp(dir / "route.tape.json"));
    {   // route.csv: the recorded positions, one row per game loop (row index == the csv's frame column)
        std::ifstream in(dir / "route.csv");
        if (!in) throw H::ReplayError("cannot open route.csv");
        std::string line;
        std::getline(in, line);
        const auto header = splitCsv(line);
        const auto column = [&](const char* name) {
            const auto it = std::find(header.begin(), header.end(), name);
            if (it == header.end()) throw H::ReplayError(std::string("route.csv has no column ") + name);
            return static_cast<std::size_t>(it - header.begin());
        };
        const auto cStage = column("stage"), cScenario = column("scenario"), cValid = column("valid"), cX = column("x"),
                   cY = column("y"), cZ = column("z"), cGrounded = column("grounded"), cDead = column("dead");
        while (std::getline(in, line)) {
            const auto f = splitCsv(line);
            if (f.size() < header.size()) continue;  // a torn final line of a killed session
            CsvRow row;
            row.stage = f[cStage];
            row.scenario = std::atoi(f[cScenario].c_str());
            row.valid = f[cValid] == "1";
            row.grounded = f[cGrounded] == "1";
            row.dead = f[cDead] == "1";
            row.x = std::strtof(f[cX].c_str(), nullptr);
            row.y = std::strtof(f[cY].c_str(), nullptr);
            row.z = std::strtof(f[cZ].c_str(), nullptr);
            mRoute.push_back(std::move(row));
        }
    }
    const std::uint64_t rows = std::min<std::uint64_t>(mMeta.seams, mRoute.size());
    if (rows < 2) throw H::ReplayError("the tape has no complete game loop");
    if (wholeSession()) {
        // Title starts at row 0. Later visits start at their first Game/End seam, including menus that never reach
        // play-ready. Only asynchronous scene construction is excluded; no menu input or level-select warp is substituted.
        std::vector<SegmentMeta> scenes(1);
        scenes[0].stage = mRoute[0].stage;
        scenes[0].scenario = mRoute[0].scenario;
        for (const auto& start : mMeta.sceneStarts) {
            if (start.startRow >= rows) continue;
            if (start.startRow == 0) scenes[0] = start;
            else {
                if (start.startRow <= scenes.back().startRow) throw H::ReplayError("unordered scene-start seams");
                scenes.push_back(start);
            }
        }
        for (std::size_t i = 0; i < scenes.size(); ++i) {
            auto& g = scenes[i];
            if (mRoute[g.startRow].stage != g.stage || mRoute[g.startRow].scenario != g.scenario)
                throw H::ReplayError("scene-start seam disagrees with route.csv");
            g.endRow = i + 1 < scenes.size() ? scenes[i + 1].startRow - 1 : rows - 1;
            // Stop before the next scene's loading rows (stage identity changes before its initialize-End anchor).
            if (i + 1 < scenes.size())
                while (g.endRow > g.startRow && (mRoute[g.endRow].stage != g.stage ||
                    (!g.stage.empty() && mRoute[g.endRow].scenario != g.scenario))) --g.endRow;
            else if (mRoute[g.startRow].valid)
                while (g.endRow > g.startRow && !mRoute[g.endRow].valid) --g.endRow;
        }
        mMeta.segments = std::move(scenes);
    }
    if (mMeta.segments.empty()) throw H::ReplayError("the tape has no play-ready segment to replay");
    // The segment plan: the same rule as tools/port/replay_tape.py plan_segments (+ trim_invalid_tail on the last segment).
    std::uint64_t offset = 0;
    for (std::size_t i = 0; i < mMeta.segments.size(); ++i) {
        const auto& g = mMeta.segments[i];
        std::uint64_t end = std::min<std::uint64_t>(g.endRow, rows - 1);
        if (!wholeSession() && i + 1 == mMeta.segments.size())
            while (end > g.startRow && !mRoute[end].valid) --end;  // a session that was quit ends in teardown loops
        if (end <= g.startRow) continue;
        mPlan.push_back({i, g.startRow, end, end - g.startRow, offset});
        offset += end - g.startRow;
    }
    if (mPlan.empty()) throw H::ReplayError("no complete game loop after a ready seam");
    {
        std::ifstream kpd(dir / "route.kpd", std::ios::binary);
        if (!kpd) throw H::ReplayError("cannot open route.kpd");
        mReplay = std::make_unique<H::Replay>(kpd, true);
        std::vector<H::Replay::SegmentRange> ranges;
        for (const auto& p : mPlan) ranges.push_back({p.start, p.end});
        mReplay->keepSegments(ranges);
    }
    if (std::ifstream actions(dir / "route.actions"); actions) {  // mod buttons: frame mod press held (absolute game loops)
        std::string line;
        while (std::getline(actions, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::istringstream words(line);
            unsigned long long frame;
            int mod, press, held;
            if (!(words >> frame >> mod >> press >> held) || mod < 0 || mod >= 16) throw H::ReplayError("bad route.actions line: " + line);
            for (const auto& p : mPlan)
                if (frame >= p.start + 1 && frame <= p.end) mActions[{p.offset + frame - (p.start + 1), mod}] = {press != 0, held != 0};
        }
    }
    for (const auto& p : mPlan) {
        SegmentReport r;
        r.index = static_cast<int>(mReport.size());
        r.stage = mMeta.segments[p.index].stage;
        r.scenario = mMeta.segments[p.index].scenario;
        r.startRow = p.start;
        r.frames = p.frames;
        mReport.push_back(std::move(r));
    }
}

void Session::compare(const SeamState& s, std::uint64_t k) {
    auto& rep = mReport[mSegIndex];
    const auto& p = mPlan[mSegIndex];
    const auto& r = mRoute[p.start + k];
    ++rep.rowsCompared;
    std::string why;
    if (s.stage != r.stage) why = "stage " + s.stage + " vs " + r.stage;
    else if (s.scenario != r.scenario) why = "scenario " + std::to_string(s.scenario) + " vs " + std::to_string(r.scenario);
    else if (s.valid != r.valid) why = std::string("valid ") + (s.valid ? "1" : "0") + " vs " + (r.valid ? "1" : "0");
    double dist = 0;
    if (s.valid && r.valid) {
        if (why.empty() && (!std::isfinite(s.x) || !std::isfinite(s.y) || !std::isfinite(s.z))) why = "nonfinite position";
        dist = std::sqrt(std::pow(double(s.x) - r.x, 2) + std::pow(double(s.y) - r.y, 2) + std::pow(double(s.z) - r.z, 2));
        if (why.empty() && s.dead != r.dead) why = "dead";
        if (why.empty() && s.grounded != r.grounded) why = "grounded";
        if (why.empty() && dist > kPositionLimit) why = "position";
    }
    rep.maxDist = std::max(rep.maxDist, dist);
    if (!why.empty() && !rep.diverged) {
        rep.diverged = true;
        rep.firstDivergenceRow = k;
        rep.firstDivergenceDist = dist;
        rep.firstDivergenceWhy = why;
        rep.firstReplayStage = s.stage;
        rep.firstRecordedStage = r.stage;
        rep.firstReplay[0] = s.x; rep.firstReplay[1] = s.y; rep.firstReplay[2] = s.z;
        rep.firstRecorded[0] = r.x; rep.firstRecorded[1] = r.y; rep.firstRecorded[2] = r.z;
        if (wholeSession()) mError = "state divergence at route row " + std::to_string(p.start + k) + ": " + why;
    }
}

Step Session::seam(const SeamState& s) {
    Step step;
    ++mSeams;
    if (mDone) return step;
    if (mError.empty()) {
        try { mReplay->endFrame(); } catch (const std::exception& e) { inputFailure(e.what(), s); }
    }
    mLastObserved = s;
    if (!mError.empty()) {  // given up: report what was reached and stop
        mDone = true;
        step.kind = Step::Kind::GaveUp;
        return step;
    }
    const auto& plan = mPlan[mSegIndex];
    if (mActive) {
        compare(s, mSegRow);
        ++mSegRow;
        if (!mError.empty()) { mDone = true; step.kind = Step::Kind::GaveUp; return step; }
        if (mReplay->segmentEnded() || mReplay->finished()) {
            mReport[mSegIndex].completed = true;
            mActive = false;
            if (++mSegIndex == mPlan.size()) {
                mDone = true;
                step.kind = Step::Kind::Finished;
            } else {
                mNeedEdge = true;   // wait for the old visit's readiness to drop
                mWarpSent = false;
                mWaitSeams = 0;
            }
        }
        return step;
    }
    // Waiting for the next segment's sync seam: its stage and scenario, play-ready (read before this seam's update).
    if (++mWaitSeams > kWaitSeamLimit) {
        if (wholeSession()) compare(s, 0);
        mError = "segment " + std::to_string(mSegIndex) + " (" + mMeta.segments[plan.index].stage + ") never reached " +
                 (wholeSession() ? "initialize-End" : "play-ready");
        mDone = true;
        step.kind = Step::Kind::GaveUp;
        return step;
    }
    const bool ready = wholeSession() ? s.initEnd : s.playReady;
    if (mNeedEdge && !ready) mNeedEdge = false;
    const auto& seg = mMeta.segments[plan.index];
    // A different stage also proves a new visit if construction completed between two seams.
    if (wholeSession() && mSegIndex > 0 && s.stage != mReport[mSegIndex - 1].stage) mNeedEdge = false;
    const bool initialTitle = wholeSession() && mSegIndex == 0 && seg.startRow == 0;
    if (!mNeedEdge && (ready || initialTitle) && s.stage == seg.stage && s.scenario == seg.scenario) {
        mActive = true;
        mSegRow = 0;
        mReport[mSegIndex].started = true;
        mReplay->startInputs();
        step.kind = Step::Kind::Start;
        step.seed = seg.seed;
        step.restoreSeed = !initialTitle || (!mMeta.sceneStarts.empty() && mMeta.sceneStarts.front().startRow == 0);
        compare(s, mSegRow);  // row 0, the sync seam, is compared with the recorded start row
        ++mSegRow;
        if (!mError.empty()) { mDone = true; step.kind = Step::Kind::GaveUp; }
        return step;
    }
    if (!wholeSession() && mSegIndex > 0 && !mWarpSent && mWaitSeams >= kWarpDelaySeams) {
        mWarpSent = true;
        step.kind = Step::Kind::Warp;
        step.stage = seg.stage;
        step.scenario = seg.scenario;
    }
    return step;
}

void Session::inputFailure(const std::string& why, const SeamState& observed) {
    if (!mError.empty()) return;
    mError = why;
    if (!wholeSession() || !mActive) return;
    auto& r = mReport[mSegIndex];
    if (r.diverged) return;
    r.diverged = true;
    r.firstDivergenceRow = mSegRow;
    r.firstDivergenceWhy = why + " (position is last observed seam; KPAD read failed before next seam)";
    const auto& recorded = mRoute[mPlan[mSegIndex].start + std::min(mSegRow, mPlan[mSegIndex].frames)];
    r.firstReplayStage = observed.stage;
    r.firstRecordedStage = recorded.stage;
    r.firstReplay[0] = observed.x; r.firstReplay[1] = observed.y; r.firstReplay[2] = observed.z;
    r.firstRecorded[0] = recorded.x; r.firstRecorded[1] = recorded.y; r.firstRecorded[2] = recorded.z;
}

std::int32_t Session::read(std::int32_t channel, std::uint32_t capacity, void* buffer, std::int32_t* error) {
    auto* out = static_cast<KPADStatus*>(buffer);
    try {
        const H::Read& input = mError.empty() ? mReplay->consume(channel, capacity) : mReplay->neutral(channel, capacity);
        for (std::size_t i = 0; i < input.samples.size(); ++i) out[i] = H::decodeSample(input.samples[i]);
        if (mActive && mError.empty() && !mReplay->waitingForSync()) {
            auto& report = mReport[mSegIndex];
            ++report.reads;
            if (channel == 0 && input.returned > 0) {
                const auto recorded = H::decodeSample(input.samples[0]);
                report.recordedPointer[0] = recorded.pos.x; report.recordedPointer[1] = recorded.pos.y;
                report.replayedPointer[0] = out[0].pos.x; report.replayedPointer[1] = out[0].pos.y;
                report.pointerValid = out[0].dpd_valid_fg;
                if (out[0].dpd_valid_fg > 0) ++report.pointingReads;
            }
        }
        if (error) *error = input.error;
        return input.returned;
    } catch (const std::exception& e) {
        inputFailure(e.what(), mLastObserved);
        const H::Read& input = mReplay->neutral(channel, capacity);
        for (std::size_t i = 0; i < input.samples.size(); ++i) out[i] = H::decodeSample(input.samples[i]);
        if (error) *error = input.error;
        return input.returned;
    }
}

bool Session::press(int mod) {
    if (mReplay->waitingForSync() || !mError.empty()) return false;
    const auto f = mReplay->frame();
    const auto it = mActions.find({f, mod});
    if (it == mActions.end() || !it->second.first) return false;
    if (mConsumedFrame != f) { mConsumedFrame = f; mConsumedMask = 0; }
    if (mConsumedMask >> mod & 1U) return false;
    mConsumedMask |= 1U << mod;
    return true;
}

bool Session::held(int mod) {
    if (mReplay->waitingForSync() || !mError.empty()) return false;
    const auto it = mActions.find({mReplay->frame(), mod});
    return it != mActions.end() && it->second.second;
}

const H::Read* Session::connection(std::uint32_t channel) const { return mReplay->connection(channel); }

std::string Session::segmentLine(std::size_t index) const {
    const auto& r = mReport[index];
    std::ostringstream out;
    out << "segment " << r.index << " " << r.stage << " scenario " << r.scenario << " (route row " << r.startRow << ", " << r.frames << " frames): ";
    if (!r.started) out << "not reached";
    else {
        out << r.rowsCompared << " rows compared" << (r.completed ? "" : " (stopped early)") << ", ";
        if (!r.diverged) out << "no divergence (max error " << r.maxDist << ")";
        else out << "first divergence at row " << r.firstDivergenceRow << " (" << r.firstDivergenceWhy << ", " << r.firstDivergenceDist
                 << "; replayed (" << r.firstReplay[0] << ", " << r.firstReplay[1] << ", " << r.firstReplay[2] << ") recorded ("
                 << r.firstRecorded[0] << ", " << r.firstRecorded[1] << ", " << r.firstRecorded[2] << ")), max error " << r.maxDist;
    }
    return out.str();
}
std::string Session::reportText() const {
    std::ostringstream out;
    out << "tape replay report (position limit " << kPositionLimit << ")\n";
    for (std::size_t i = 0; i < mReport.size(); ++i) out << segmentLine(i) << "\n";
    if (!mError.empty()) out << "replay stopped: " << mError << "\n";
    return out.str();
}
std::string Session::reportJson() const {
    std::ostringstream out;
    out << "{\n  \"mode\": " << jsonString(wholeSession() ? "whole-session" : "segments")
        << ",\n  \"position_limit\": " << kPositionLimit << ",\n  \"finished\": " << (mDone && mError.empty() ? "true" : "false")
        << ",\n  \"stopped\": " << jsonString(mError) << ",\n  \"segments\": [";
    for (std::size_t i = 0; i < mReport.size(); ++i) {
        const auto& r = mReport[i];
        out << (i ? "," : "") << "\n    {\"segment\": " << r.index << ", \"stage\": " << jsonString(r.stage) << ", \"scenario\": " << r.scenario
            << ", \"route_start_row\": " << r.startRow << ", \"tape_frames\": " << r.frames << ", \"started\": " << (r.started ? "true" : "false")
            << ", \"completed\": " << (r.completed ? "true" : "false") << ", \"rows_compared\": " << r.rowsCompared
            << ", \"matched_rows\": " << (r.diverged ? r.firstDivergenceRow : r.rowsCompared) << ", \"diverged\": " << (r.diverged ? "true" : "false")
            << ", \"first_divergence_row\": " << (r.diverged ? std::to_string(r.firstDivergenceRow) : std::string("null"))
            << ", \"first_divergence_why\": " << jsonString(r.firstDivergenceWhy) << ", \"first_divergence_dist\": " << r.firstDivergenceDist
            << ", \"first_divergence_frame\": " << (r.diverged ? std::to_string(r.startRow + r.firstDivergenceRow) : "null")
            << ", \"first_divergence_stage_replayed\": " << jsonString(r.firstReplayStage)
            << ", \"first_divergence_stage_recorded\": " << jsonString(r.firstRecordedStage)
            << ", \"first_divergence_replayed\": [" << r.firstReplay[0] << ", " << r.firstReplay[1] << ", " << r.firstReplay[2]
            << "], \"first_divergence_recorded\": [" << r.firstRecorded[0] << ", " << r.firstRecorded[1] << ", " << r.firstRecorded[2] << "]"
            << ", \"reads\": " << r.reads << ", \"pointing_reads\": " << r.pointingReads
            << ", \"pointer_source\": \"recorded KPAD and delivered KPAD, not screen coordinates\""
            << ", \"last_pointer_recorded\": [" << r.recordedPointer[0] << ", " << r.recordedPointer[1] << "]"
            << ", \"last_pointer_replayed\": [" << r.replayedPointer[0] << ", " << r.replayedPointer[1] << "]"
            << ", \"last_pointer_valid\": " << r.pointerValid
            << ", \"max_position_error\": " << r.maxDist << "}";
    }
    out << "\n  ]\n}\n";
    return out.str();
}

}  // namespace PetariNative::App::TapePlay
