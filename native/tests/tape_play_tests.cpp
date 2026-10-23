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
        sample[92] = 1; sample[95] = 5;
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
    fs::remove_all(base);
    std::printf("tape play session checks passed (%d)\n", checks);
}
