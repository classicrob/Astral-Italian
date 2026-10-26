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
#include <iomanip>
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
    if (const auto* schema = root.find("schema")) {
        if (schema->type != Json::Type::Number || (schema->number != 1 && schema->number != 2))
            throw H::ReplayError("unsupported tape schema (expected 1 or 2)");
        meta.schema = static_cast<unsigned>(schema->number);
    }
    const auto number = [&](const Json& obj, const char* key) {
        const Json* v = obj.find(key);
        if (!v || v->type != Json::Type::Number || v->number < 0) throw H::ReplayError(std::string("bad tape sidecar: missing ") + key);
        return static_cast<std::uint64_t>(v->number);
    };
    if (meta.schema != 2 && root.find("neutral_tail_policy"))
        throw H::ReplayError("neutral-tail policy requires schema2 session-control protocol");
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
            seg.sceneReady = true;
            meta.sceneStarts.push_back(std::move(seg));
        }
    if (const Json* starts = root.find("play_ready_seams"))
        for (const auto& r : starts->items) {
            SegmentMeta seg;
            seg.startRow = number(r, "row");
            seg.stage = r.find("stage") ? r.find("stage")->text : std::string();
            seg.scenario = static_cast<int>(number(r, "scenario"));
            seg.seed = static_cast<std::uint32_t>(number(r, "rng_seed"));
            meta.playStarts.push_back(std::move(seg));
        }
    if (meta.schema == 2) {
        const auto require = [](bool value, const char* why) { if (!value) throw H::ReplayError(why); };
        const auto integer = [&](const Json& j, const char* key) -> std::uint64_t {
            const auto* v = j.find(key);
            require(v && v->type == Json::Type::Number && std::isfinite(v->number) && v->number >= 0 &&
                    std::floor(v->number) == v->number && v->number <= 10000000, "invalid schema2 frame/id");
            return static_cast<std::uint64_t>(v->number);
        };
        const auto string = [&](const Json& j, const char* key) {
            const auto* v = j.find(key); require(v && v->type == Json::Type::String, "missing schema2 string"); return v->text;
        };
        const auto boolean = [&](const Json& j, const char* key) {
            const auto* v = j.find(key); require(v && v->type == Json::Type::Bool, "missing schema2 boolean"); return v->boolean;
        };
        const auto state = [&](const Json& j) {
            TapePhase::State s;
            s.inputFrame = integer(j,"input_frame"); s.kind = string(j,"kind");
            s.scene = string(j,"scene"); s.stage = string(j,"stage");
            const auto* scenario = j.find("scenario");
            require(scenario && scenario->type == Json::Type::Number && std::floor(scenario->number)==scenario->number &&
                    scenario->number >= -1 && scenario->number <= 10000, "invalid schema2 scenario");
            s.scenario = static_cast<int>(scenario->number);
            s.firstNormal = boolean(j,"first_normal_update"); s.valid = boolean(j,"valid");
            s.grounded = boolean(j,"grounded"); s.dead = boolean(j,"dead");
            s.rngObserved = boolean(j,"rng_observed"); s.mapWait = boolean(j,"map_wait"); s.galaxyMap = boolean(j,"galaxy_map");
            const auto* seed = j.find("rng_seed");
            require(seed && seed->type == Json::Type::Number && seed->number>=0 && seed->number<=UINT32_MAX &&
                    std::floor(seed->number)==seed->number, "invalid schema2 RNG");
            s.rng = static_cast<std::uint32_t>(seed->number);
            const auto* bits = j.find("position_bits");
            require(bits && bits->type == Json::Type::Array && bits->items.size()==3, "missing schema2 position bits");
            for (unsigned i=0;i<3;++i) {
                const auto& b=bits->items[i];
                require(b.type==Json::Type::Number && b.number>=0 && b.number<=UINT32_MAX && std::floor(b.number)==b.number,
                        "invalid schema2 position bits");
                s.position[i]=static_cast<std::uint32_t>(b.number);
                float f; std::memcpy(&f,&s.position[i],4); require(std::isfinite(f),"nonfinite schema2 marker position");
            }
            return s;
        };
        require(string(root,"marker_protocol")==TapePhase::kProtocol, "unsupported schema2 marker protocol");
        require(string(root,"marker_error").empty(), "recorder rejected schema2 marker path");
        const auto* launch=root.find("launch");
        require(launch && launch->type==Json::Type::Object,"schema2 launch metadata missing");
        meta.launchStage=string(*launch,"stage");
        meta.launchScenario=static_cast<int>(integer(*launch,"scenario"));
        require(meta.launchScenario>0 && meta.launchScenario<=10000,"invalid schema2 launch scenario");
        const auto* phases=root.find("input_phases");
        require(phases && phases->type==Json::Type::Array && !phases->items.empty(), "schema2 requires game-defined input phases");
        for (const auto& j:phases->items) {
            TapePhase::Phase p; p.id=integer(j,"id");
            const auto first=integer(j,"first_input_frame"); p.endInputFrame=integer(j,"end_input_frame_exclusive");
            const auto endKind=string(j,"end_kind");
            require(endKind=="phase-transition" || endKind=="recording-prefix", "invalid schema2 end kind");
            p.open=endKind=="recording-prefix";
            const auto* marker=j.find("marker"); require(marker && marker->type==Json::Type::Object,"schema2 marker missing");
            p.marker=state(*marker);
            require(p.id==meta.inputPhases.size() && first==p.marker.inputFrame && first>0 &&
                    p.endInputFrame>first && p.endInputFrame<=meta.seams, "unordered/out-of-prefix schema2 phase");
            require(!p.open || p.endInputFrame==meta.seams,"schema2 open phase must cover the declared prefix end");
            require(p.marker.kind=="controller-normal" || p.marker.kind=="scenario-foreground", "unknown schema2 phase kind");
            if (!meta.inputPhases.empty())
                require(!meta.inputPhases.back().open && first>=meta.inputPhases.back().endInputFrame, "overlapping schema2 phases");
            if (p.marker.kind=="controller-normal" && !p.marker.firstNormal)
                require(!meta.inputPhases.empty() && meta.inputPhases.back().marker.kind=="scenario-foreground",
                        "Normal marker is neither first update nor overlay resume");
            const auto* points=j.find("checkpoints");
            require(points && points->type==Json::Type::Array,"schema2 checkpoints missing");
            for (const auto& point:points->items) {
                TapePhase::Checkpoint c; c.kind=string(point,"kind");
                const auto* ps=point.find("state"); require(ps && ps->type==Json::Type::Object,"schema2 checkpoint state missing");
                c.state=state(*ps);
                require(c.state.key()==p.marker.key() && c.state.inputFrame>=first && c.state.inputFrame<p.endInputFrame,
                        "schema2 checkpoint outside phase");
                require((c.kind=="map-wait" && c.state.mapWait) || (c.kind=="galaxy-map" && c.state.galaxyMap),
                        "invalid schema2 checkpoint predicate");
                require(p.checkpoints.empty() || c.state.inputFrame>=p.checkpoints.back().state.inputFrame,
                        "unordered schema2 checkpoints");
                for (const auto& existing:p.checkpoints)
                    require(c.state.inputFrame!=existing.state.inputFrame || c.kind!=existing.kind,
                            "duplicate schema2 checkpoint");
                p.checkpoints.push_back(std::move(c));
            }
            meta.inputPhases.push_back(std::move(p));
        }
        if (root.find("session_control_protocol")) {
            // Duplicate fields make event order ambiguous even if a first-key
            // JSON accessor could otherwise choose one silently.
            const auto unique = [&](const auto& self,const Json& value)->void {
                std::set<std::string> names;
                for (const auto& item:value.members) {
                    require(names.insert(item.first).second,"duplicate session-control JSON field");
                    self(self,item.second);
                }
                for (const auto& item:value.items) self(self,item);
            };
            unique(unique,root);
            require(string(root,"session_control_protocol")==TapePower::kProtocol,"unsupported session-control protocol");
            require(string(root,"session_control_error").empty(),"recorder rejected session-control ordering");
            meta.powerProtocol=true;
            if (root.find("neutral_tail_policy"))
                meta.neutralTailPolicy=TapeNeutral::parse(string(root,"neutral_tail_policy"));
            const auto* events=root.find("session_events");
            require(events && events->type==Json::Type::Array && events->items.size()<=2,"missing/ambiguous session events");
            for (const auto& j:events->items) {
                TapePower::Event event;
                event.id=integer(j,"id");event.phase=integer(j,"phase_id");
                event.afterRow=integer(j,"after_route_row");event.beforeInputFrame=integer(j,"before_input_frame");
                event.kind=string(j,"kind");event.callbackObserved=boolean(j,"callback_observed");
                require(event.id==meta.sessionEvents.size() && string(j,"boundary")==TapePower::kBoundary &&
                    event.beforeInputFrame==event.afterRow+1 && event.phase<meta.inputPhases.size(),"ambiguous power-event boundary");
                const auto& phase=meta.inputPhases[event.phase];
                require(event.afterRow>=phase.marker.inputFrame && event.afterRow<phase.endInputFrame &&
                    event.afterRow<meta.seams-1,"session event outside its phase/prefix");
                const auto* elapsed=j.find("neutral_elapsed_seconds");
                require(elapsed && elapsed->type==Json::Type::Number && std::isfinite(elapsed->number) && elapsed->number>=0,
                    "invalid neutral-tail clock observation");
                event.neutralElapsedSeconds=elapsed->number;
                require((event.kind=="neutral-tail-begin" && event.id==0 && !event.callbackObserved) ||
                    (event.kind=="power-request" && event.callbackObserved && event.phase==meta.inputPhases.size()-1),
                    "unknown/unacknowledged session event");
                if (!meta.sessionEvents.empty())
                    require(meta.sessionEvents.back().kind=="neutral-tail-begin" && event.kind=="power-request" &&
                        event.afterRow>meta.sessionEvents.back().afterRow && event.phase==meta.sessionEvents.back().phase,
                        "duplicate or unordered session controls");
                meta.sessionEvents.push_back(std::move(event));
            }
            if (meta.complete) require(!meta.sessionEvents.empty() && meta.sessionEvents.back().kind=="power-request",
                "complete power session missing power-request event");
        } else {
            require(!root.find("session_events") && !root.find("session_control_error") && !root.find("neutral_tail_policy"),
                    "session events/policy lack versioned protocol");
        }
    }
    return meta;
}

Session::Session(const std::filesystem::path& dir, Mode mode) : mMode(mode) {
    mMeta = parseSidecar(slurp(dir / "route.tape.json"));
    if (const char* path=std::getenv("PETARI_TAPE_PLAY_READ_MAP"); path && *path) {
        if (std::filesystem::exists(path)) throw H::ReplayError("read mapping output already exists");
        const auto parent=std::filesystem::path(path).parent_path();
        if (!parent.empty()) std::filesystem::create_directories(parent);
        mReadMap.open(path); if (!mReadMap) throw H::ReplayError("cannot create read mapping output");
    }
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
    if (mMeta.powerProtocol && (!wholeSession() || !mMeta.complete || mRoute.size()!=mMeta.seams))
        throw H::ReplayError("power-session proof requires complete whole-session route, including shutdown");
    const std::uint64_t rows = std::min<std::uint64_t>(mMeta.seams, mRoute.size());
    if (rows < 2) throw H::ReplayError("the tape has no complete game loop");
    if (mMeta.schema == 2 && wholeSession()) { initializePhases(dir,rows); return; }
    if (wholeSession()) {
        // Game/End anchors cover FileSelect, helm and map, but schema 1 has no ready checkpoint for Logo/Title or
        // ScenarioSelect (stage=the destination, scenario=-1). Those visits must also feed their inputs: anchor on the
        // scene identity assignment. Initial setup and asynchronous construction/teardown around Game/End are neutral.
        std::vector<SegmentMeta> scenes;
        for (std::uint64_t begin = 0; begin < rows;) {
            std::uint64_t end = begin;
            while (end + 1 < rows && mRoute[end + 1].stage == mRoute[begin].stage &&
                   mRoute[end + 1].scenario == mRoute[begin].scenario) ++end;
            const bool setup = begin == 0 && mRoute[begin].stage.empty() && mRoute[begin].scenario == 1 &&
                end + 1 < rows && mRoute[end + 1].stage.empty() && mRoute[end + 1].scenario == -1;
            const auto first = scenes.size();
            if (!setup) {
                for (const auto& checkpoint : mMeta.sceneStarts) {
                    if (checkpoint.startRow < begin || checkpoint.startRow > end) continue;
                    if (checkpoint.stage != mRoute[begin].stage || checkpoint.scenario != mRoute[begin].scenario)
                        throw H::ReplayError("scene-start seam disagrees with route.csv");
                    if (scenes.size() > first) {
                        if (checkpoint.startRow <= scenes.back().startRow) throw H::ReplayError("unordered scene-start seams");
                        scenes.back().endRow = checkpoint.startRow - 1;
                    }
                    scenes.push_back(checkpoint);
                    scenes.back().endRow = end;
                }
                if (scenes.size() == first) {
                    SegmentMeta identity;
                    identity.startRow = begin; identity.endRow = end;
                    identity.stage = mRoute[begin].stage; identity.scenario = mRoute[begin].scenario;
                    identity.hasSeed = false;
                    scenes.push_back(std::move(identity));
                }
                for (std::size_t i = first; i < scenes.size(); ++i) {
                    auto& g = scenes[i];
                    g.identityRow = begin;
                    if (mRoute[g.startRow].valid)
                        while (g.endRow > g.startRow && !mRoute[g.endRow].valid) --g.endRow;
                }
            }
            begin = end + 1;
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
        if (wholeSession())
            for (const auto& ready : mMeta.playStarts)
                if (ready.startRow >= g.startRow && ready.startRow <= end && ready.stage == g.stage && ready.scenario == g.scenario) {
                    if (ready.startRow == end) throw H::ReplayError("no complete gameplay loop after play-ready");
                    mPlan.back().hasPlayReady = true;
                    mPlan.back().playRow = ready.startRow;
                    mPlan.back().playSeed = ready.seed;
                    break;
                }
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
        r.sceneStartRow = p.start;
        r.identityRecorded = mMeta.segments[p.index].identityRow;
        r.requiresPlayReady = p.hasPlayReady;
        r.rngRecorded = p.playSeed;
        r.startRow = p.hasPlayReady ? p.playRow : p.start;
        r.frames = p.end - r.startRow;
        mReport.push_back(std::move(r));
    }
}

std::int64_t Session::expectedIdentity(std::size_t index) const {
    std::int64_t shift = 0;
    if (index > 0) {
        const auto& previous = mReport[index - 1];
        // Measure each boundary on the preceding scene's last interactive clock, rather than accumulating
        // unrelated load/cinematic shifts over the entire session.
        const auto actual = previous.requiresPlayReady ? previous.replayPlayStart : previous.replaySceneStart;
        shift = static_cast<std::int64_t>(actual) - static_cast<std::int64_t>(previous.startRow);
    }
    return static_cast<std::int64_t>(mReport[index].identityRecorded) + shift;
}

bool Session::observeIdentity(std::size_t index, const SeamState& s) {
    auto& r = mReport[index];
    if (r.identityObserved || s.stage != r.stage || s.scenario != r.scenario) return true;
    r.identityObserved = true;
    r.identityReplay = mSeams - 1;
    r.identityExpected = expectedIdentity(index);
    r.identityOffset = static_cast<std::int64_t>(r.identityReplay) - r.identityExpected;
    if (std::abs(r.identityOffset) > kIdentityOffsetLimit) {
        inputFailure("scene identity offset exceeds 30 loops for " + r.stage, s);
        return false;
    }
    return true;
}

void Session::compare(const SeamState& s, std::uint64_t k, bool bitExact) {
    auto& rep = mReport[mSegIndex];
    const auto& r = mRoute[rep.startRow + k];
    bool constructionBoundary = false;
    if (wholeSession() && !bitExact && mSegIndex + 1 < mPlan.size() &&
        (s.stage != r.stage || s.scenario != r.scenario)) {
        const auto next = mSegIndex + 1;
        const auto& target = mMeta.segments[mPlan[next].index];
        const auto& evidence = mReport[next];
        const auto tapeRow = rep.startRow + k;
        // Only the known next scene's async construction may overlap the old identity tail. Its logic has not
        // begun; the recorded read shape and actual game pointer are still checked for this loop below.
        constructionBoundary = target.sceneReady && evidence.identityObserved && !s.initEnd && !s.valid &&
            s.stage == target.stage && s.scenario == target.scenario && tapeRow < target.identityRow &&
            target.identityRow - tapeRow <= static_cast<std::uint64_t>(kIdentityOffsetLimit) &&
            std::abs(evidence.identityOffset) <= kIdentityOffsetLimit;
    }
    if (constructionBoundary) ++rep.identityRowsSkipped;
    else ++rep.rowsCompared;
    std::string why;
    if (constructionBoundary) {} // different construction identities: no per-frame state comparison
    else if (s.stage != r.stage) why = "stage " + s.stage + " vs " + r.stage;
    else if (s.scenario != r.scenario) why = "scenario " + std::to_string(s.scenario) + " vs " + std::to_string(r.scenario);
    else if (s.valid != r.valid) why = std::string("valid ") + (s.valid ? "1" : "0") + " vs " + (r.valid ? "1" : "0");
    if (why.empty() && bitExact && (!s.valid || !r.valid)) why = "invalid player at play-ready";
    if (wholeSession() && k > 0 && mPointerReadMade) {
        ++rep.pointerRowsCompared;
        rep.replayedPointer[0] = s.pointerX; rep.replayedPointer[1] = s.pointerY;
        if (why.empty() && (s.pointerObserved != mExpectedPointer || (mExpectedPointer &&
            (s.pointerValid != mExpectedPointerValid || s.pointerX != mExpectedPointerX || s.pointerY != mExpectedPointerY))))
            why = "KPAD pointer in game read buffer";
    }
    double dist = 0;
    if (!constructionBoundary && s.valid && r.valid) {
        if (why.empty() && (!std::isfinite(s.x) || !std::isfinite(s.y) || !std::isfinite(s.z))) why = "nonfinite position";
        dist = std::sqrt(std::pow(double(s.x) - r.x, 2) + std::pow(double(s.y) - r.y, 2) + std::pow(double(s.z) - r.z, 2));
        if (why.empty() && s.dead != r.dead) why = "dead";
        if (why.empty() && s.grounded != r.grounded) why = "grounded";
        if (why.empty() && (bitExact || wholeSession()) && (std::memcmp(&s.x, &r.x, sizeof(float)) ||
            std::memcmp(&s.y, &r.y, sizeof(float)) || std::memcmp(&s.z, &r.z, sizeof(float))))
            why = bitExact ? "position bits at play-ready" : "position bits";
        if (why.empty() && dist > kPositionLimit) why = "position";
    }
    if (why.empty() && !constructionBoundary && wholeSession() && k > 0 && mPointerReadMade &&
        s.valid && r.valid && mExpectedPointer && s.pointerObserved && mExpectedPointerValid && s.pointerValid) {
        ++rep.pointerValidRowsCompared;
        rep.pointerPositionsCompared.emplace(s.pointerX, s.pointerY);
    }
    rep.maxDist = std::max(rep.maxDist, dist);
    if (!why.empty() && !rep.diverged) {
        rep.diverged = true;
        rep.firstDivergenceRow = k;
        rep.firstDivergenceTapeRow = rep.startRow + k;
        rep.firstDivergenceDist = dist;
        rep.firstDivergenceWhy = why;
        rep.firstReplayStage = s.stage;
        rep.firstRecordedStage = r.stage;
        rep.firstReplay[0] = s.x; rep.firstReplay[1] = s.y; rep.firstReplay[2] = s.z;
        rep.firstRecorded[0] = r.x; rep.firstRecorded[1] = r.y; rep.firstRecorded[2] = r.z;
        if (wholeSession()) mError = "state divergence at route row " + std::to_string(rep.startRow + k) + ": " + why;
    }
}

Step Session::startPlay(const SeamState& s) {
    const auto& plan = mPlan[mSegIndex];
    auto& report = mReport[mSegIndex];
    report.playReadyChecked = true;
    report.playState = s;
    report.replayPlayStart = mSeams - 1;
    report.cinematicFrames = report.replayPlayStart - report.replaySceneStart;
    report.playReplay[0] = s.x; report.playReplay[1] = s.y; report.playReplay[2] = s.z;
    report.rngNatural = s.rngSeed;
    report.rngRecorded = plan.playSeed;
    const auto& recorded = mRoute[plan.playRow];
    report.playReadyPositionExact = !std::memcmp(&s.x, &recorded.x, sizeof(float)) &&
        !std::memcmp(&s.y, &recorded.y, sizeof(float)) && !std::memcmp(&s.z, &recorded.z, sizeof(float));
    mPointerReadMade = false; // the boundary checks state, before any reads from the re-anchored gameplay stream
    compare(s, 0, true);
    report.playReadyStateMatches = !report.diverged;
    if (!mError.empty()) { mDone = true; return {Step::Kind::GaveUp}; }
    mReplay->seekForwardFrame(plan.offset + plan.playRow - plan.start);
    mStrict = true;
    mSegRow = 1;
    mPendingSeed = true;
    mConsumedFrame = UINT64_MAX;
    Step step;
    step.kind = Step::Kind::Start;
    step.seed = plan.playSeed;
    return step;
}

bool Session::acknowledgeSeed(std::uint32_t seed) {
    if (mPendingMarkerSeed) {
        mPendingMarkerSeed=false;
        auto& e=mPhaseEvidence[mSegIndex]; e.rngReadback=seed;
        e.rngVerified=seed==mMeta.inputPhases[mSegIndex].marker.rng;
        if (!e.rngVerified) phaseFailure("schema2 marker RNG read-back differs");
        return e.rngVerified;
    }
    if (!mPendingSeed) return mError.empty();
    mPendingSeed = false;
    auto& r = mReport[mSegIndex];
    r.rngReadback = seed;
    r.rngCheckpointVerified = seed == r.rngRecorded;
    if (!r.rngCheckpointVerified) {
        r.diverged = true;
        r.firstDivergenceRow = 0;
        r.firstDivergenceTapeRow = r.startRow;
        r.firstDivergenceWhy = "RNG checkpoint read-back at play-ready";
        r.firstReplayStage = r.firstRecordedStage = r.stage;
        for (int i = 0; i < 3; ++i) { r.firstReplay[i] = r.playReplay[i]; r.firstRecorded[i] = r.playReplay[i]; }
        mError = r.firstDivergenceWhy;
        mDone = true;
    }
    return r.rngCheckpointVerified;
}

Step Session::seam(const SeamState& s) {
    if (mMeta.schema==2 && wholeSession()) {
        if (mMeta.powerProtocol && mDone && mError.empty()) {
            phaseFailure("extra seam after declared shutdown end");++mSeams;return {Step::Kind::GaveUp};
        }
        auto step=phaseSeam(s);
        if (mMeta.powerProtocol && mError.empty()) {
            const auto control=controlSeam(s);
            if (control.kind==Step::Kind::GaveUp) return control;
            step.neutralTailBegin=control.neutralTailBegin;step.powerRequest=control.powerRequest;
        }
        return step;
    }
    Step step;
    ++mSeams;
    mActualOrdinal=0;
    if (mDone) return step;
    if (mPendingSeed) {
        inputFailure("play-ready RNG checkpoint was not acknowledged", s);
        mDone = true;
        return {Step::Kind::GaveUp};
    }
    if (mError.empty()) {
        try { mReplay->endFrame(); } catch (const std::exception& e) { inputFailure(e.what(), s); }
    }
    mLastObserved = s;
    if (!mError.empty()) {  // given up: report what was reached and stop
        mDone = true;
        step.kind = Step::Kind::GaveUp;
        return step;
    }
    if (wholeSession()) {
        if (!observeIdentity(mSegIndex, s) || (mActive && mSegIndex + 1 < mPlan.size() &&
            !observeIdentity(mSegIndex + 1, s))) {
            mDone = true;
            return {Step::Kind::GaveUp};
        }
        if (!mActive && !mReport[mSegIndex].identityObserved &&
            static_cast<std::int64_t>(mSeams - 1) > expectedIdentity(mSegIndex) + kIdentityOffsetLimit) {
            inputFailure("scene identity not reached within 30 loops for " + mReport[mSegIndex].stage, s);
            mDone = true;
            return {Step::Kind::GaveUp};
        }
    }
    const auto& plan = mPlan[mSegIndex];
    if (mActive) {
        if (!mStrict) {
            if (s.playReady) return startPlay(s);
            ++mSegRow;
            if (++mWaitSeams > kWaitSeamLimit || s.stage != mMeta.segments[plan.index].stage ||
                s.scenario != mMeta.segments[plan.index].scenario) {
                inputFailure("scene never reached its recorded play-ready seam", s);
                mDone = true;
                return {Step::Kind::GaveUp};
            }
            // A longer host cinematic gets neutral input after the recorded cinematic reads end. Gameplay reads begin
            // only after the actual play-ready state passes; a shorter cinematic skips only the remaining arrival reads.
            if (mReplay->frame() >= plan.offset + plan.playRow - plan.start) mReplay->pauseInputs();
            return step;
        }
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
    const auto& seg = mMeta.segments[plan.index];
    const bool ready = wholeSession() ? (!seg.sceneReady || s.initEnd) : s.playReady;
    if (mNeedEdge && !ready) mNeedEdge = false;
    // A different stage also proves a new visit if construction completed between two seams.
    if (wholeSession() && mSegIndex > 0 && (s.stage != mReport[mSegIndex - 1].stage ||
        s.scenario != mReport[mSegIndex - 1].scenario)) mNeedEdge = false;
    if (!mNeedEdge && ready && s.stage == seg.stage && s.scenario == seg.scenario) {
        mActive = true;
        mExpectedPointer = false;
        mPointerReadMade = false;
        mSegRow = 0;
        mReport[mSegIndex].started = true;
        mReport[mSegIndex].replaySceneStart = mSeams - 1;
        mStrict = !plan.hasPlayReady;
        mReplay->startInputs();
        step.kind = Step::Kind::Start;
        step.seed = seg.seed;
        step.restoreSeed = !wholeSession() || seg.hasSeed;
        if (mStrict) compare(s, mSegRow);  // menus compare immediately; arrivals compare only at play-ready
        else {
            if (s.playReady) return startPlay(s);
            if (plan.playRow == plan.start) mReplay->pauseInputs();
        }
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
    r.firstDivergenceTapeRow = mStrict ? r.startRow + std::min(mSegRow, r.frames)
                                     : mPlan[mSegIndex].start + std::min(mSegRow, mPlan[mSegIndex].frames);
    r.firstDivergenceWhy = why + " (position is last observed seam; KPAD read failed before next seam)";
    const auto& recorded = mRoute[r.firstDivergenceTapeRow];
    r.firstReplayStage = observed.stage;
    r.firstRecordedStage = recorded.stage;
    r.firstReplay[0] = observed.x; r.firstReplay[1] = observed.y; r.firstReplay[2] = observed.z;
    r.firstRecorded[0] = recorded.x; r.firstRecorded[1] = recorded.y; r.firstRecorded[2] = recorded.z;
}

std::int32_t Session::read(std::int32_t channel, std::uint32_t capacity, void* buffer, std::int32_t* error) {
    auto* out = static_cast<KPADStatus*>(buffer);
    try {
        if (mPendingMarkerSeed) phaseFailure("schema2 input before marker RNG acknowledgement");
        const bool recorded = !mDone && mError.empty() && !mReplay->waitingForSync();
        const H::Read& input = !mDone && mError.empty() ? mReplay->consume(channel, capacity) : mReplay->neutral(channel, capacity);
        validateNeutralTail(input);
        recordReadMap(input,recorded);
        if (mShutdownTail && recorded) ++mShutdownReads;
        ++mActualOrdinal;
        for (std::size_t i = 0; i < input.samples.size(); ++i) out[i] = H::decodeSample(input.samples[i]);
        if (mActive && mError.empty() && !mReplay->waitingForSync()) {
            auto& report = mReport[mSegIndex];
            ++report.reads;
            if (channel == 0) {
                mPointerReadMade = true;
                mExpectedPointer = input.returned > 0;
            }
            if (channel == 0 && input.returned > 0) {
                const auto recorded = H::decodeSample(input.samples[0]);
                report.recordedPointer[0] = recorded.pos.x; report.recordedPointer[1] = recorded.pos.y;
                report.replayedPointer[0] = out[0].pos.x; report.replayedPointer[1] = out[0].pos.y;
                report.pointerValid = out[0].dpd_valid_fg;
                mExpectedPointer = true;
                mExpectedPointerX = recorded.pos.x; mExpectedPointerY = recorded.pos.y;
                mExpectedPointerValid = recorded.dpd_valid_fg;
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

#include "tape_phase_replay.inc"
#include "tape_power_replay.inc"

bool Session::press(int mod) {
    if (mDone || mReplay->waitingForSync() || !mError.empty()) return false;
    const auto f = mReplay->frame();
    const auto it = mActions.find({f, mod});
    if (it == mActions.end() || !it->second.first) return false;
    if (mConsumedFrame != f) { mConsumedFrame = f; mConsumedMask = 0; }
    if (mConsumedMask >> mod & 1U) return false;
    mConsumedMask |= 1U << mod;
    return true;
}

bool Session::held(int mod) {
    if (mDone || mReplay->waitingForSync() || !mError.empty()) return false;
    const auto it = mActions.find({mReplay->frame(), mod});
    return it != mActions.end() && it->second.second;
}

const H::Read* Session::connection(std::uint32_t channel) const {
    if (channel >= 4) return nullptr;
    // Must select the SAME source as read(), including neutral post-roll and
    // stopped replay, not the last recorded frame's stale connection.
    if (mDone || !mError.empty()) return &mReplay->neutral(channel, 1);
    return mReplay->connection(channel);
}

void Observation::update(double elapsed, std::uint64_t row, bool replaySucceeded) {
    if (requestedSeconds <= 0 || completed) return;
    elapsedSeconds = elapsed;
    if (!replaySucceeded) return;
    if (!postStarted) {
        postStarted = true;
        postStartSeconds = elapsed;
        postStartRow = row;
    }
    endRow = row;
    if (elapsed >= requestedSeconds && elapsed-postStartSeconds >= minimumPostSeconds) {
        completed = true;
        stopSeconds = elapsed;
    }
}

std::string Observation::reportJson(double startMonotonicSeconds) const {
    std::ostringstream out;
    out << std::setprecision(17) << "{\"requested_seconds\":" << requestedSeconds
        << ",\"minimum_post_roll_seconds\":" << minimumPostSeconds
        << ",\"clock\":\"steady_clock; quit scheduling only\",\"start_monotonic_seconds\":" << startMonotonicSeconds
        << ",\"stop_monotonic_seconds\":" << (completed ? std::to_string(startMonotonicSeconds + stopSeconds) : "null")
        << ",\"elapsed_seconds\":" << elapsedSeconds << ",\"completed\":" << (completed ? "true" : "false")
        << ",\"post_roll\":{\"started\":" << (postStarted ? "true" : "false")
        << ",\"start_route_row\":" << (postStarted ? std::to_string(postStartRow) : "null")
        << ",\"end_route_row\":" << (postStarted ? std::to_string(endRow) : "null")
        << ",\"start_elapsed_seconds\":" << (postStarted ? std::to_string(postStartSeconds) : "null")
        << ",\"elapsed_seconds\":" << (postStarted ? elapsedSeconds - postStartSeconds : 0)
        << ",\"loops\":" << (postStarted ? endRow - postStartRow : 0)
        << ",\"input\":\"neutral\",\"input_source\":" << jsonString(recordedTail ? "recorded-session-neutral-tail; exact raw sample check" : "Headless::Replay::neutral")
        << ",\"neutral_read_calls\":" << neutralReads << "}}";
    return out.str();
}

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
    out << std::setprecision(9);
    out << "{\n  \"mode\": " << jsonString(wholeSession() ? "whole-session" : "segments")
        << ",\n  \"tape_schema\": " << mMeta.schema
        << ",\n  \"tape_format_label\": " << jsonString(mMeta.schema==1 ? "legacy-v1" : mMeta.powerProtocol ? "game-phase-v2-power-v1" : "game-phase-v2-legacy-no-power")
        << ",\n  \"legacy_policy\": " << (mMeta.schema==1 ? "true" : "false")
        << ",\n  \"anchor_policy\": " << jsonString(wholeSession() ? (mMeta.schema==2 ? TapePhase::kPolicy : "interactive-and-play-ready-v1") : "legacy-play-ready")
            << ",\n  \"position_limit\": " << kPositionLimit
            << ",\n  \"interactive_position_bit_exact\": " << (wholeSession() ? "true" : "false")
            << ",\n  \"finished\": " << (mDone && mError.empty() ? "true" : "false")
        << ",\n  \"stopped\": " << jsonString(mError) << ",\n  \"segments\": [";
    for (std::size_t i = 0; i < mReport.size(); ++i) {
        const auto& r = mReport[i];
        const auto& p = mPlan[i];
        const auto stateJson = [&](const auto& s) {
            const auto bits = [](float f) { std::uint32_t b; std::memcpy(&b, &f, 4); return b; };
            out << "{\"stage\":" << jsonString(s.stage) << ",\"scenario\":" << s.scenario
                << ",\"valid\":" << (s.valid ? "true" : "false") << ",\"grounded\":" << (s.grounded ? "true" : "false")
                << ",\"dead\":" << (s.dead ? "true" : "false") << ",\"position\":[" << s.x << ',' << s.y << ',' << s.z
                << "],\"position_bits\":[" << bits(s.x) << ',' << bits(s.y) << ',' << bits(s.z) << "]}";
        };
        out << (i ? "," : "") << "\n    {\"segment\": " << r.index << ", \"stage\": " << jsonString(r.stage) << ", \"scenario\": " << r.scenario
            << ", \"anchor\": " << jsonString(!wholeSession() ? "play-ready" : mMeta.schema==2 ? "game-defined-pre-read" : mMeta.segments[mPlan[i].index].sceneReady ? "initialize-End" : "scene-identity")
            << ", \"rng_checkpoint_available\": " << (mMeta.segments[mPlan[i].index].hasSeed ? "true" : "false")
            << ", \"scene_anchor_recorded_frame\": " << r.sceneStartRow
            << ", \"scene_anchor_replay_frame\": " << (r.started ? std::to_string(r.replaySceneStart) : "null")
            << ", \"scene_anchor_frame_offset\": " << (r.started ? std::to_string(static_cast<std::int64_t>(r.replaySceneStart) - static_cast<std::int64_t>(r.sceneStartRow)) : "null")
            << ", \"identity_recorded_frame\": " << r.identityRecorded
            << ", \"identity_replay_frame\": " << (r.identityObserved ? std::to_string(r.identityReplay) : "null")
            << ", \"identity_expected_replay_frame\": " << (r.identityObserved ? std::to_string(r.identityExpected) : "null")
            << ", \"identity_frame_offset\": " << (r.identityObserved ? std::to_string(r.identityOffset) : "null")
            << ", \"identity_offset_limit\": " << kIdentityOffsetLimit
            << ", \"identity_offset_checked\": " << (r.identityObserved ? "true" : "false")
            << ", \"identity_offset_passed\": " << (r.identityObserved && std::abs(r.identityOffset) <= kIdentityOffsetLimit ? "true" : "false")
            << ", \"identity_rows_skipped\": " << r.identityRowsSkipped
            << ", \"comparison_anchor\": " << jsonString(!wholeSession() || r.requiresPlayReady ? "play-ready" : mMeta.schema==2 ? "pre-read-scene" : "scene-start")
            << ", \"play_ready_required\": " << (r.requiresPlayReady ? "true" : "false")
            << ", \"route_start_row\": " << r.startRow << ", \"tape_frames\": " << r.frames << ", \"started\": " << (r.started ? "true" : "false")
            << ", \"completed\": " << (r.completed ? "true" : "false") << ", \"rows_compared\": " << r.rowsCompared
            << ", \"matched_rows\": " << (r.diverged ? r.firstDivergenceRow : r.rowsCompared) << ", \"diverged\": " << (r.diverged ? "true" : "false")
            << ", \"first_divergence_row\": " << (r.diverged ? std::to_string(r.firstDivergenceRow) : std::string("null"))
            << ", \"first_divergence_why\": " << jsonString(r.firstDivergenceWhy) << ", \"first_divergence_dist\": " << r.firstDivergenceDist
            << ", \"first_divergence_frame\": " << (r.diverged ? std::to_string(r.firstDivergenceTapeRow) : "null")
            << ", \"first_divergence_stage_replayed\": " << jsonString(r.firstReplayStage)
            << ", \"first_divergence_stage_recorded\": " << jsonString(r.firstRecordedStage)
            << ", \"first_divergence_replayed\": [" << r.firstReplay[0] << ", " << r.firstReplay[1] << ", " << r.firstReplay[2]
            << "], \"first_divergence_recorded\": [" << r.firstRecorded[0] << ", " << r.firstRecorded[1] << ", " << r.firstRecorded[2] << "]"
            << ", \"reads\": " << r.reads << ", \"pointing_reads\": " << r.pointingReads
            << ", \"pointer_rows_compared\": " << r.pointerRowsCompared
            << ", \"pointer_valid_rows_compared\": " << r.pointerValidRowsCompared
            << ", \"pointer_distinct_positions_compared\": " << r.pointerPositionsCompared.size()
            << ", \"pointer_source\": \"recorded KPAD versus game WPad read buffer, not screen coordinates\""
            << ", \"last_pointer_recorded\": [" << r.recordedPointer[0] << ", " << r.recordedPointer[1] << "]"
            << ", \"last_pointer_replayed\": [" << r.replayedPointer[0] << ", " << r.replayedPointer[1] << "]"
            << ", \"last_pointer_valid\": " << r.pointerValid
            << ", \"max_position_error\": " << r.maxDist
            << ", \"play_ready_state_check\": ";
        if (!r.requiresPlayReady) out << "null";
        else {
            out << "{\"checked\":" << (r.playReadyChecked ? "true" : "false")
                << ",\"passed\":" << (r.playReadyStateMatches && r.rngCheckpointVerified ? "true" : "false")
                << ",\"recorded_frame\":" << p.playRow
                << ",\"replay_frame\":" << (r.playReadyChecked ? std::to_string(r.replayPlayStart) : "null")
                << ",\"frame_offset\":" << (r.playReadyChecked ? std::to_string(static_cast<std::int64_t>(r.replayPlayStart) - static_cast<std::int64_t>(p.playRow)) : "null")
                << ",\"arrival_recorded_frames\":" << p.playRow - p.start
                << ",\"arrival_replay_frames\":" << r.cinematicFrames
                << ",\"arrival_frame_offset\":" << (r.playReadyChecked ? std::to_string(static_cast<std::int64_t>(r.cinematicFrames) - static_cast<std::int64_t>(p.playRow - p.start)) : "null")
                << ",\"position_bit_exact\":" << (r.playReadyPositionExact ? "true" : "false") << ",\"expected\":";
            stateJson(mRoute[p.playRow]);
            out << ",\"replayed\":";
            if (r.playReadyChecked) stateJson(r.playState); else out << "null";
            out << ",\"rng\":{\"recorded\":" << r.rngRecorded << ",\"natural_before_restore\":" << r.rngNatural
                << ",\"natural_observed\":" << (r.playState.rngObserved ? "true" : "false")
                << ",\"readback_after_restore\":" << r.rngReadback
                << ",\"verified\":" << (r.rngCheckpointVerified ? "true" : "false") << "}}";
        }
        out << "}";
    }
    out << "\n  ]";
    if (mMeta.powerProtocol) {
        const auto required=mMeta.seams-mMeta.sessionEvents.back().beforeInputFrame;
        out << ",\n  \"session_control\":{\"protocol\":" << jsonString(TapePower::kProtocol)
            << ",\"neutral_tail_policy\":" << jsonString(TapeNeutral::name(mMeta.neutralTailPolicy))
            << ",\"events_required\":" << mMeta.sessionEvents.size() << ",\"events_delivered\":" << mControlActualRows.size()
            << ",\"power_callback_verified\":" << (mPowerDelivered ? "true":"false")
            << ",\"power_reached_early\":" << (mEarlyPower ? "true":"false")
            << ",\"power_actual_after_route_row\":" << (mPowerDelivered ? std::to_string(mPowerActualRow):"null")
            << ",\"shutdown_reads_compared\":" << mShutdownReads
            << ",\"shutdown_rows_required\":" << required << ",\"shutdown_rows_compared\":" << mShutdownRows
            << ",\"shutdown_position_bit_exact\":" << (mError.empty() && mShutdownRows==required ? "true":"false")
            << ",\"normal_exit_verified\":" << (mNormalExitVerified ? "true":"false") << ",\"events\":[";
        for (std::size_t i=0;i<mMeta.sessionEvents.size();++i) {
            const auto& event=mMeta.sessionEvents[i];
            out << (i ? ",":"") << "{\"id\":" << i << ",\"kind\":" << jsonString(event.kind)
                << ",\"phase_id\":" << event.phase << ",\"recorded_after_route_row\":" << event.afterRow
                << ",\"actual_after_route_row\":" << (i<mControlActualRows.size() ? std::to_string(mControlActualRows[i]):"null") << '}';
        }
        out << "]}";
    }
    if (mMeta.schema==2 && wholeSession()) {
        out << ",\n  \"marker_protocol\":" << jsonString(TapePhase::kProtocol)
            << ",\n  \"non_interactive_gap_loops\":" << mNeutralLoops << ",\n  \"input_phases\":[";
        for (std::size_t i=0;i<mPhaseEvidence.size();++i) {
            const auto& e=mPhaseEvidence[i]; const auto& p=mMeta.inputPhases[i];
            out << (i ? "," : "") << "{\"id\":" << p.id << ",\"marker_checked\":" << (e.markerChecked ? "true":"false")
                << ",\"marker_passed\":" << (e.markerPassed ? "true":"false")
                << ",\"rng_verified\":" << (e.rngVerified ? "true":"false")
                << ",\"recorded_marker\":" << TapePhase::stateJson(p.marker)
                << ",\"actual_marker\":" << (e.markerChecked ? TapePhase::stateJson(e.actualMarker) : "null")
                << ",\"frame_offset\":" << (e.markerChecked ? std::to_string(static_cast<std::int64_t>(e.actualMarker.inputFrame)-static_cast<std::int64_t>(p.marker.inputFrame)) : "null")
                << ",\"rng_natural_before_restore\":" << e.rngNatural << ",\"rng_readback\":" << e.rngReadback
                << ",\"end_input_frame_exclusive\":" << p.endInputFrame
                << ",\"exit_required\":" << (p.open ? "false":"true")
                << ",\"exit_checked\":" << (e.exitChecked ? "true":"false")
                << ",\"exit_passed\":" << (e.exitPassed ? "true":"false")
                << ",\"checkpoints_required\":" << p.checkpoints.size()
                << ",\"checkpoints_checked\":" << e.checkpointsChecked << '}';
        }
        out << ']';
    }
    out << "\n}\n";
    return out.str();
}

}  // namespace PetariNative::App::TapePlay
