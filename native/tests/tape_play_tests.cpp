// Tape replay session (native/app/tape_play.hpp), game-free: a synthetic two-segment tape in a temp directory is driven seam by
// seam the way the GX frame seam drives it; reads, mod buttons, re-anchoring, the warp to the next segment, the divergence
// report and the give-up path are checked.
#include "tape_play.hpp"

#include <revolution/kpad.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

#include <unistd.h>

using namespace PetariNative::App::TapePlay;
namespace H = PetariNative::Headless;
namespace fs = std::filesystem;

namespace {
int checks = 0;
void check(bool ok, const char* what) {
    ++checks;
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", what); std::exit(1); }
}
void number(std::ostream& out, std::uint64_t v, unsigned n) { for (unsigned i = n; i > 0; --i) out.put(static_cast<char>(v >> (8 * (i - 1)))); }

constexpr unsigned kRows = 20;
// Segment 0: rows 2..6 (4 frames), segment 1: rows 10..13 (3 frames); the read of absolute frame f has hold = 0x100 + f.
void writeTape(const fs::path& dir, double wrongX = -1) {
    fs::create_directories(dir);
    std::ofstream(dir / "route.tape.json") <<
        "{\n \"schema\": 1, \"complete\": true, \"seams\": 20, \"exe_sha256\": \"abc\", \"mods\": [\"OdysseyJumpDive\"],\n"
        " \"segments\": [\n  {\"start_row\": 2, \"end_row\": 6, \"frames\": 4, \"stage\": \"TestGalaxy\", \"scenario\": 1, \"rng_seed\": 111},\n"
        "  {\"start_row\": 10, \"end_row\": 13, \"frames\": 3, \"stage\": \"TestGalaxy\", \"scenario\": 1, \"rng_seed\": 222}\n ]\n}\n";
    {
        std::ofstream csv(dir / "route.csv");
        csv << "frame,stage,scenario,zone_ground_id,valid,x,y,z,grounded,dead\n";
        for (unsigned r = 0; r < kRows; ++r) csv << r << ",\"TestGalaxy\",1,-1,1," << r << ".5,2,3,1,0\n";
    }
    std::ofstream(dir / "route.actions") << "# frame mod press held\n4 0 1 1\n5 0 0 1\n11 1 1 0\n";
    std::ofstream kpd(dir / "route.kpd", std::ios::binary);
    kpd << "SMG2KPD1";
    number(kpd, kRows, 8);
    number(kpd, kRows, 8);
    for (unsigned f = 0; f < kRows; ++f) {
        number(kpd, f, 8); number(kpd, 0, 4); number(kpd, 0, 4); number(kpd, 120, 4);
        number(kpd, 1, 4); number(kpd, 0, 4); number(kpd, 0, 4); number(kpd, 1, 4); number(kpd, 1, 4);
        H::Sample sample{};
        sample[0] = 0; sample[1] = 0; sample[2] = static_cast<std::uint8_t>(f >> 8 | 1); sample[3] = static_cast<std::uint8_t>(f);
        const float pointer = f * 0.01f;
        std::uint32_t pointerBits;
        std::memcpy(&pointerBits, &pointer, 4);
        for (unsigned i = 0; i < 4; ++i) sample[32 + i] = pointerBits >> (24 - 8 * i);
        sample[92] = 1; sample[94] = 1; sample[95] = 5;
        kpd.write(reinterpret_cast<const char*>(sample.data()), sample.size());
    }
    (void)wrongX;
}

SeamState state(unsigned row, bool ready, float x = -1) {
    SeamState s;
    s.stage = "TestGalaxy"; s.scenario = 1; s.valid = true; s.grounded = true;
    s.x = x < 0 ? row + 0.5f : x; s.y = 2; s.z = 3; s.playReady = ready;
    return s;
}
unsigned hold(Session& s, int channel = 0, std::uint32_t capacity = 120) {
    KPADStatus buffer[120] = {};
    std::int32_t error = 99;
    const auto n = s.read(channel, capacity, buffer, &error);
    check(n == 1 && error == 0, "a read returns one sample, no error");
    return buffer[0].hold;
}
}  // namespace

int main() {
    {   // Untimestamped same-channel attach+detach in one frame must remain
        // fail-closed before playback: never flatten it to one connection.
        std::stringstream data;
        data << "SMG2KPD1";number(data,1,8);number(data,2,8);
        for (unsigned ordinal=0;ordinal<2;++ordinal) {
            number(data,0,8);number(data,ordinal,4);number(data,0,4);number(data,120,4);
            number(data,ordinal==0?1:0,4);number(data,ordinal==0?0:static_cast<std::uint32_t>(-2),4);
            number(data,ordinal==0?0:static_cast<std::uint32_t>(-1),4);number(data,ordinal==0?1:253,4);number(data,1,4);
            H::Sample sample{};data.write(reinterpret_cast<const char*>(sample.data()),sample.size());
        }
        bool rejected=false;
        try { H::Replay replay(data); }
        catch (const H::ReplayError& error) {
            rejected=std::strstr(error.what(),"within-frame connection transitions")!=nullptr;
        }
        check(rejected,"same-frame attach+detach rejected before callback/sample delivery");
    }
    const fs::path base = fs::temp_directory_path() / ("tape_play_tests_" + std::to_string(::getpid()));
    fs::remove_all(base);
    writeTape(base / "tape");
    {   // the sidecar parser
        std::ifstream in(base / "tape/route.tape.json");
        std::stringstream text; text << in.rdbuf();
        const auto meta = parseSidecar(text.str());
        check(meta.seams == 20 && meta.complete && meta.exeSha256 == "abc" && meta.mods.size() == 1, "sidecar header");
        check(meta.segments.size() == 2 && meta.segments[1].startRow == 10 && meta.segments[1].seed == 222, "sidecar segments");
        bool threw = false;
        try { parseSidecar("{\"seams\": 3"); } catch (const H::ReplayError&) { threw = true; }
        check(threw, "a torn sidecar is rejected");
    }
    Session session(base / "tape");
    check(session.segmentCount() == 2 && session.firstSegment().stage == "TestGalaxy", "plan");
    // Before the first segment: neutral reads, no start while play-ready is false or the stage differs.
    check(hold(session) == 0, "neutral read before the first segment");
    check(session.seam(state(0, false)).kind == Step::Kind::None && session.seam(state(1, false)).kind == Step::Kind::None, "waiting");
    check(!session.press(0) && !session.held(0), "no mod buttons before the first segment");
    auto other = state(2, true); other.stage = "Elsewhere";
    check(session.seam(other).kind == Step::Kind::None, "play-ready on another stage does not start");
    // Segment 0 starts at the first matching play-ready seam; its recorded reads (absolute frames 3..6) follow.
    auto step = session.seam(state(2, true));
    check(step.kind == Step::Kind::Start && step.seed == 111, "segment 0 starts with its RNG seed");
    for (unsigned k = 1; k <= 4; ++k) {
        const unsigned frame = 2 + k;  // the read of absolute loop 3.. : hold = 0x100 | frame
        check(hold(session) == (0x100u | frame), "recorded read of the loop");
        if (k == 2) check(session.press(0) == (frame == 4) && session.held(0), "mod press");
        step = session.seam(state(2 + k, true));
        check(k < 4 ? step.kind == Step::Kind::None : step.kind == Step::Kind::None, "segment runs");
    }
    check(session.report()[0].completed && !session.report()[0].diverged && session.report()[0].rowsCompared == 5, "segment 0 matched 5 rows");
    check(hold(session) == 0, "neutral between segments");
    // The next segment is the same stage and scenario: it must not start until play-ready dropped (a new visit), and the glue is
    // asked once to warp there.
    for (std::uint64_t i = 1; i < kWarpDelaySeams; ++i) check(session.seam(state(7, true)).kind == Step::Kind::None, "the game gets time to get there itself");
    step = session.seam(state(7, true));
    check(step.kind == Step::Kind::Warp && step.stage == "TestGalaxy" && step.scenario == 1, "warp to the next segment");
    check(session.seam(state(8, true)).kind == Step::Kind::None, "no second warp and no start in the old visit");
    check(session.seam(state(9, false)).kind == Step::Kind::None, "visit changed");
    step = session.seam(state(10, true));
    check(step.kind == Step::Kind::Start && step.seed == 222, "segment 1 starts after a new play-ready edge");
    // Segment 1 drifts at its second row (tape x differs by 0.5): reported with the row index.
    for (unsigned k = 1; k <= 3; ++k) {
        hold(session);
        step = session.seam(state(10 + k, true, k >= 2 ? 10 + k + 0.5f + 0.5f : -1));
    }
    check(step.kind == Step::Kind::Finished && session.finished(), "tape finished");
    const auto& r = session.report()[1];
    check(r.diverged && r.firstDivergenceRow == 2 && r.firstDivergenceWhy == "position" && r.rowsCompared == 4, "divergence at row 2");
    check(r.maxDist > 0.49 && r.maxDist < 0.51, "divergence size");
    check(session.reportText().find("first divergence at row 2") != std::string::npos, "report text");
    check(session.reportJson().find("\"first_divergence_row\": 2") != std::string::npos, "report json");

    {   // A game that reads differently than recorded: the replay is given up (and says why); reads go neutral.
        Session s2(base / "tape");
        s2.seam(state(2, true));
        KPADStatus buffer[16] = {};
        std::int32_t error = 0;
        s2.read(0, 16, buffer, &error);  // recorded capacity is 120
        check(s2.gaveUp() && s2.error().find("mismatch") != std::string::npos, "capacity mismatch gives up");
        check(s2.seam(state(3, true)).kind == Step::Kind::GaveUp, "give-up is reported");
        check(s2.reportText().find("replay stopped") != std::string::npos, "give-up in the report");
    }
    {   // A tape without a segment cannot be replayed.
        fs::create_directories(base / "bad");
        std::ofstream(base / "bad/route.tape.json") << "{\"seams\": 5, \"segments\": []}";
        bool threw = false;
        try { Session s3(base / "bad"); } catch (const H::ReplayError&) { threw = true; }
        check(threw, "no segment");
    }
    {   // A whole session has menu visits with no player and no play-ready segment at all.
        const auto dir = base / "menus";
        writeTape(dir);
        std::ofstream(dir / "route.tape.json") << R"({"complete":true,"seams":20,"segments":[],"ready_seams":[
            {"row":4,"stage":"FileSelect","scenario":1,"rng_seed":123},
            {"row":11,"stage":"WorldMap01Galaxy","scenario":1,"rng_seed":456}]})";
        {
            std::ofstream csv(dir / "route.csv");
            csv << "frame,stage,scenario,valid,x,y,z,grounded,dead\n";
            for (unsigned row = 0; row < kRows; ++row)
                csv << row << ',' << (row < 3 ? "" : row < 10 ? "FileSelect" : "WorldMap01Galaxy")
                    << ',' << (row == 1 || row == 2 ? -1 : 1) << ",0,0,0,0,0,0,0\n";
        }
        const auto menu = [](const char* stage, bool ready) {
            SeamState s; s.stage = stage; s.scenario = 1; s.initEnd = ready;
            s.pointerObserved = true; s.pointerValid = 1; return s;
        };
        Session menus(dir, Session::Mode::WholeSession);
        check(menus.segmentCount() == 3 && menus.wholeSession(), "whole session plans title and non-play-ready menus");
        check(menus.seam(menu("", false)).kind == Step::Kind::None, "system setup is outside the first scene anchor");
        auto titleState = menu("", false); titleState.scenario = -1;
        const auto title = menus.seam(titleState);
        check(title.kind == Step::Kind::Start && !title.restoreSeed, "title anchors before play-ready without inventing an RNG checkpoint");
        for (unsigned frame = 2; frame <= 2; ++frame) {
            check(hold(menus) == (0x100u | frame), "title input is replayed");
            auto s = titleState; s.pointerX = frame * 0.01f; menus.seam(s);
        }
        for (unsigned i = 0; i < kWarpDelaySeams + 2; ++i)
            check(menus.seam(menu("FileSelect", false)).kind == Step::Kind::None, "loading waits without warping");
        check(hold(menus) == 0, "neutral only while constructing a scene");
        auto step = menus.seam(menu("FileSelect", true));
        check(step.kind == Step::Kind::Start && step.seed == 123, "FileSelect anchors at initialize-End without play-ready");
        for (unsigned frame = 5; frame <= 9; ++frame) {
            check(hold(menus) == (0x100u | frame), "FileSelect reads stay in recorded order");
            auto s = menu("FileSelect", true); s.pointerX = frame * 0.01f; menus.seam(s);
        }
        menus.seam(menu("WorldMap01Galaxy", false));
        step = menus.seam(menu("WorldMap01Galaxy", true));
        check(step.kind == Step::Kind::Start && step.seed == 456, "map anchors without play-ready");
        for (unsigned frame = 12; frame <= 19; ++frame) {
            check(hold(menus) == (0x100u | frame), "map reads are delivered");
            auto s = menu("WorldMap01Galaxy", true); s.pointerX = frame * 0.01f; step = menus.seam(s);
        }
        check(step.kind == Step::Kind::Finished && !menus.gaveUp(), "whole menu session finishes");
        check(menus.report()[0].reads == 1 && menus.report()[1].reads == 5 && menus.report()[2].reads == 8,
              "all menu input frames counted");
        check(menus.report()[2].pointingReads == 8 && menus.report()[2].replayedPointer[0] == 19 * 0.01f
            && menus.report()[2].recordedPointer[0] == menus.report()[2].replayedPointer[0]
            && menus.report()[2].pointerRowsCompared == 8, "map pointer samples match the game read buffer exactly");
        Session bad(dir, Session::Mode::WholeSession);
        bad.seam(titleState);
        hold(bad);
        check(bad.seam(menu("WrongScene", true)).kind == Step::Kind::GaveUp, "whole session fails closed at first scene divergence");
        check(bad.report()[0].firstDivergenceRow == 1 && bad.report()[0].firstReplayStage == "WrongScene",
              "first divergence retains row and both stages");
        check(hold(bad) == 0 && bad.error().find("route row 2") != std::string::npos, "failed session stops feeding recorded input");
        Session wrongRead(dir, Session::Mode::WholeSession);
        wrongRead.seam(titleState);
        KPADStatus buffer[16] = {};
        wrongRead.read(0, 16, buffer, nullptr);
        check(wrongRead.gaveUp() && wrongRead.report()[0].diverged
              && wrongRead.report()[0].firstDivergenceRow == 1, "input-shape divergence retains absolute frame evidence");
        check(wrongRead.error().find("recorded frame=0") != std::string::npos
              && wrongRead.error().find("replayed channel=0 capacity=16") != std::string::npos, "read failure records expected and actual shape");
        Session pointerDrift(dir, Session::Mode::WholeSession);
        pointerDrift.seam(titleState); hold(pointerDrift);
        check(pointerDrift.seam(titleState).kind == Step::Kind::GaveUp
              && pointerDrift.report()[0].firstDivergenceWhy == "KPAD pointer in game read buffer", "pointer mismatch fails closed");
        // ScenarioSelect has a destination stage and scenario -1, but schema-1 never gives it a Game/End checkpoint.
        std::ofstream(dir / "route.tape.json") << R"({"complete":true,"seams":20,"segments":[],"ready_seams":[
            {"row":4,"stage":"FileSelect","scenario":1,"rng_seed":123},
            {"row":16,"stage":"WorldMap01Galaxy","scenario":1,"rng_seed":456}]})";
        {
            std::ofstream csv(dir / "route.csv");
            csv << "frame,stage,scenario,valid,x,y,z,grounded,dead\n";
            for (unsigned row = 0; row < kRows; ++row)
                csv << row << ',' << (row < 3 ? "" : row < 10 ? "FileSelect" : "WorldMap01Galaxy")
                    << ',' << (row == 1 || row == 2 || (row >= 10 && row < 14) ? -1 : 1) << ",0,0,0,0,0,0,0\n";
        }
        Session scenarioSelect(dir, Session::Mode::WholeSession);
        check(scenarioSelect.segmentCount() == 4, "scenario select is retained without a ready checkpoint");
        scenarioSelect.seam(titleState); hold(scenarioSelect);
        auto titleEnd = titleState; titleEnd.pointerX = 2 * 0.01f; scenarioSelect.seam(titleEnd);
        scenarioSelect.seam(menu("FileSelect", true));
        for (unsigned f = 5; f <= 9; ++f) {
            hold(scenarioSelect); auto s = menu("FileSelect", true); s.pointerX = f * 0.01f; scenarioSelect.seam(s);
        }
        auto selecting = menu("WorldMap01Galaxy", false); selecting.scenario = -1;
        step = scenarioSelect.seam(selecting);
        check(step.kind == Step::Kind::Start && !step.restoreSeed, "scenario select starts on its identity, without invented RNG");
        for (unsigned f = 11; f <= 13; ++f) {
            check(hold(scenarioSelect) == (0x100u | f), "scenario selection reads are replayed");
            auto s = selecting; s.pointerX = f * 0.01f; scenarioSelect.seam(s);
        }
        check(scenarioSelect.seam(menu("WorldMap01Galaxy", false)).kind == Step::Kind::None, "same-stage scenario loading waits");
        step = scenarioSelect.seam(menu("WorldMap01Galaxy", true));
        check(step.kind == Step::Kind::Start && step.restoreSeed && step.seed == 456, "scenario 1 anchors after scenario selection");
    }
    {   // Direct --stage boots construct FileSelect asynchronously before entering the requested galaxy map.
        const auto dir = base / "direct-stage";
        writeTape(dir);
        std::ofstream(dir / "route.tape.json") << R"({"complete":true,"seams":20,"segments":[],"ready_seams":[
            {"row":10,"stage":"FileSelect","scenario":1,"rng_seed":123},
            {"row":15,"stage":"WorldMap01Galaxy","scenario":1,"rng_seed":456}]})";
        {
            std::ofstream csv(dir / "route.csv");
            csv << "frame,stage,scenario,valid,x,y,z,grounded,dead\n";
            for (unsigned row = 0; row < kRows; ++row)
                csv << row << ',' << (row < 8 ? "" : row < 13 ? "FileSelect" : "WorldMap01Galaxy")
                    << ',' << (row > 0 && row < 8 ? -1 : 1) << ",0,0,0,0,0,0,0\n";
        }
        const auto menu = [](const char* stage, bool ready, unsigned readFrame = 0) {
            SeamState s; s.stage = stage; s.scenario = stage[0] ? 1 : -1; s.initEnd = ready;
            s.pointerObserved = true; s.pointerValid = 1; s.pointerX = readFrame * 0.01f; return s;
        };
        for (const int offset : {-1, 1, 30}) {
            Session direct(dir, Session::Mode::WholeSession);
            auto setup = menu("", false); setup.scenario = 1; direct.seam(setup);
            direct.seam(menu("", false));
            for (unsigned f = 2; f <= 7; ++f) {
                check(hold(direct) == (0x100u | f), "direct-stage title reads remain authoritative");
                direct.seam(menu(offset < 0 && f == 7 ? "FileSelect" : "", false, f));
            }
            // The old segment still checks its last actual game pointer buffer when only the identity differs.
            const unsigned identity = 8 + offset;
            for (unsigned row = 8; row < 10 + offset; ++row) {
                check(hold(direct) == 0, "direct-stage construction waits neutrally");
                direct.seam(menu(row >= identity ? "FileSelect" : "", false));
            }
            auto step = direct.seam(menu("FileSelect", true));
            check(step.kind == Step::Kind::Start && step.seed == 123, "direct-stage FileSelect still anchors at init End");
            for (unsigned f = 11; f <= 12; ++f) {
                hold(direct); direct.seam(menu("FileSelect", true, f));
            }
            direct.seam(menu("WorldMap01Galaxy", false));
            direct.seam(menu("WorldMap01Galaxy", false));
            step = direct.seam(menu("WorldMap01Galaxy", true));
            check(step.kind == Step::Kind::Start && step.seed == 456, "direct-stage map still anchors at init End");
            for (unsigned f = 16; f <= 19; ++f) {
                hold(direct); step = direct.seam(menu("WorldMap01Galaxy", true, f));
            }
            check(step.kind == Step::Kind::Finished && !direct.gaveUp(), "bounded early/late direct boot finishes");
            check(direct.report()[1].identityObserved && direct.report()[1].identityOffset == offset,
                  "direct-stage identity offset is recorded relative to previous interactive anchor");
            check(direct.report()[0].identityRowsSkipped == (offset < 0 ? 1u : 0u) &&
                  direct.report()[0].pointerRowsCompared == 6, "only one-sided identity state is excluded; every pointer remains checked");
            check(direct.report()[2].identityOffset == 0, "identity bound does not accumulate previous init timing shifts");
        }
        Session late(dir, Session::Mode::WholeSession);
        auto setup = menu("", false); setup.scenario = 1; late.seam(setup); late.seam(menu("", false));
        for (unsigned f = 2; f <= 7; ++f) { hold(late); late.seam(menu("", false, f)); }
        for (unsigned row = 8; row <= 38; ++row) late.seam(menu("", false));
        check(late.seam(menu("FileSelect", false)).kind == Step::Kind::GaveUp &&
              late.error().find("identity offset exceeds 30") != std::string::npos, "31-loop identity offset fails closed");
        Session absent(dir, Session::Mode::WholeSession);
        absent.seam(setup); absent.seam(menu("", false));
        for (unsigned f = 2; f <= 7; ++f) { hold(absent); absent.seam(menu("", false, f)); }
        for (unsigned row = 8; row <= 39; ++row) absent.seam(menu("", false));
        check(absent.gaveUp() && absent.error().find("identity not reached within 30") != std::string::npos,
              "missing next identity cannot wait past the fixed bound");
        Session pointer(dir, Session::Mode::WholeSession);
        pointer.seam(setup); pointer.seam(menu("", false));
        for (unsigned f = 2; f <= 6; ++f) { hold(pointer); pointer.seam(menu("", false, f)); }
        hold(pointer);
        check(pointer.seam(menu("FileSelect", false)).kind == Step::Kind::GaveUp &&
              pointer.report()[0].firstDivergenceWhy == "KPAD pointer in game read buffer",
              "identity overlap cannot hide pointer divergence");
    }
    {   // Non-interactive arrivals may take fewer/more loops; exact play-ready is the hard boundary.
        const auto dir = base / "arrival";
        writeTape(dir);
        std::ofstream(dir / "route.tape.json") << R"({"complete":true,"seams":20,"segments":[],
            "ready_seams":[{"row":2,"stage":"TestGalaxy","scenario":1,"rng_seed":111}],
            "play_ready_seams":[{"row":6,"stage":"TestGalaxy","scenario":1,"rng_seed":222}]})";
        const auto observed = [](unsigned row, bool ready) {
            auto s = state(row, ready); s.initEnd = true;
            s.pointerObserved = true; s.pointerValid = 1; s.pointerX = row * 0.01f;
            s.rngObserved = true; s.rngSeed = 999; return s;
        };
        Session early(dir, Session::Mode::WholeSession);
        auto step = early.seam(observed(2, false));
        check(step.kind == Step::Kind::Start && step.seed == 111, "arrival reads anchor at scene-ready");
        check(hold(early) == 0x103u, "arrival receives recorded input before play-ready");
        step = early.seam(observed(6, true));
        check(step.kind == Step::Kind::Start && step.seed == 222, "short arrival re-anchors to recorded play-ready");
        check(early.acknowledgeSeed(222), "restored RNG checkpoint is read back");
        check(early.report()[0].rowsCompared == 1 && early.report()[0].playReadyPositionExact,
              "only exact play-ready starts fidelity comparison");
        for (unsigned f = 7; f < kRows; ++f) {
            check(hold(early) == (0x100u | f), "gameplay reads re-anchor without replaying skipped arrival input");
            step = early.seam(observed(f, true));
        }
        check(step.kind == Step::Kind::Finished && !early.gaveUp(), "short arrival completes all strict gameplay rows");
        check(early.report()[0].rowsCompared == 14 && early.report()[0].pointerRowsCompared == 13,
              "comparison and pointer counts exclude only the arrival");
        check(early.report()[0].pointerValidRowsCompared == 13 && early.report()[0].pointerPositionsCompared.size() == 13,
              "valid pointer rows and distinct positions come from successful actual game read-buffer comparisons");
        check(hold(early, 0, 16) == 0 && !early.press(0) && !early.held(0) && !early.gaveUp(),
              "successful post-roll returns neutral KPAD/mod input without consuming past the tape");
        early.seam(observed(7, true));
        check(early.report()[0].rowsCompared == 14 && !early.gaveUp(), "post-roll cannot alter completed fidelity evidence");
        check(early.reportJson().find("\"arrival_frame_offset\":-3") != std::string::npos,
              "report preserves the short arrival offset");

        Session late(dir, Session::Mode::WholeSession);
        late.seam(observed(2, false));
        for (unsigned f = 3; f <= 6; ++f) {
            check(hold(late) == (0x100u | f), "long arrival consumes recorded arrival reads in order");
            auto unstable = observed(2, false); unstable.x = 1000; unstable.grounded = false;
            check(late.seam(unstable).kind == Step::Kind::None && !late.gaveUp(),
                  "non-interactive cinematic positions are not compared per frame");
        }
        for (unsigned wait = 0; wait < 2; ++wait) {
            check(hold(late) == 0, "long arrival waits neutrally before gameplay input");
            late.seam(observed(2, false));
        }
        check(hold(late) == 0, "recorded gameplay input is held until actual play-ready");
        step = late.seam(observed(6, true));
        check(step.kind == Step::Kind::Start && late.acknowledgeSeed(222), "long arrival passes exact state and RNG boundary");
        check(late.reportJson().find("\"arrival_frame_offset\":3") != std::string::npos, "long arrival offset is reported");
        check(hold(late) == 0x107u, "long arrival resumes at the exact first gameplay read");
        late.seam(observed(7, true));

        Session badPosition(dir, Session::Mode::WholeSession);
        badPosition.seam(observed(2, false)); hold(badPosition);
        auto drift = observed(6, true); drift.x += 0.001f;
        check(badPosition.seam(drift).kind == Step::Kind::GaveUp &&
              badPosition.report()[0].firstDivergenceWhy == "position bits at play-ready",
              "play-ready rejects even sub-tolerance float-bit differences");
        Session badGameplay(dir, Session::Mode::WholeSession);
        badGameplay.seam(observed(2, false)); hold(badGameplay); badGameplay.seam(observed(6, true));
        badGameplay.acknowledgeSeed(222); hold(badGameplay);
        auto gameplayDrift = observed(7, true); gameplayDrift.x += 0.001f;
        check(badGameplay.seam(gameplayDrift).kind == Step::Kind::GaveUp &&
              badGameplay.report()[0].firstDivergenceWhy == "position bits",
              "every compared interactive position rejects sub-tolerance float-bit differences");
        Session badSeed(dir, Session::Mode::WholeSession);
        badSeed.seam(observed(2, false)); hold(badSeed); badSeed.seam(observed(6, true));
        check(!badSeed.acknowledgeSeed(223) && badSeed.gaveUp(), "wrong RNG read-back fails closed");
        Session missingSeed(dir, Session::Mode::WholeSession);
        missingSeed.seam(observed(2, false)); hold(missingSeed); missingSeed.seam(observed(6, true));
        hold(missingSeed);
        check(missingSeed.seam(observed(7, true)).kind == Step::Kind::GaveUp,
              "missing RNG read-back cannot continue gameplay");
    }
    {
        Observation observation; observation.requestedSeconds = 90;
        observation.update(10, 500, false);
        check(!observation.postStarted && !observation.completed, "unfinished/failed replay cannot start successful post-roll");
        observation.update(30, 1800, true);
        check(observation.pending() && observation.postStartRow == 1800, "post-roll begins only after successful replay");
        observation.update(89.99, 5399, true);
        check(!observation.completed, "observation cannot finish before the full wall-clock duration");
        observation.update(90.01, 5401, true);
        check(observation.completed && !observation.pending() && observation.endRow - observation.postStartRow == 3601,
              "wall time controls quit only and post-roll loop extent is recorded");
        observation.update(91, 5460, true);
        check(observation.elapsedSeconds == 90.01, "observation stop is frozen before graceful shutdown");
        check(observation.reportJson(100).find("\"input\":\"neutral\"") != std::string::npos,
              "observation report declares the authoritative neutral input source");
    }
    fs::remove_all(base);
    std::printf("tape play session checks passed (%d)\n", checks);
}
