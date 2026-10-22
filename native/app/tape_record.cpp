#include "tape_record.hpp"

#include <petari/kpad_record.hpp>
#include <petari/mods.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <vector>

#ifdef __APPLE__
#include <CommonCrypto/CommonDigest.h>
#include <mach-o/dyld.h>
#endif

namespace PetariNative::App::TapeRecord {
namespace {

std::string gRoute, gBase;
bool gInit = false, gOpen = false, gBackground = false;
std::uint64_t gRow = 0;
std::vector<Ready> gReadys, gPlayReadys;
bool gWasPlayReady = false;
// One tape segment per scene visit that reached play-ready: it starts at the visit's first play-ready seam (row `start`; tape
// frame 0 = the loop after it) and ends at the visit's last seam (row `end`, inclusive); the loops between segments (scene
// teardown, loading, the opening flyover) are not part of the tape: a replay re-anchors at each segment's own sync seam.
struct Segment {
    std::uint64_t start, end;
    std::string stage;
    int scenario;
    std::uint32_t seed;
    bool open;
};
std::vector<Segment> gSegments;
bool gVisitHasSegment = false;
std::string gSceneKey;  // scene visit identity at the last seam

std::string exeSha256() {
#ifdef __APPLE__
    char path[4096];
    std::uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) return "unavailable";
    std::FILE* f = std::fopen(path, "rb");
    if (!f) return "unavailable";
    CC_SHA256_CTX ctx;
    CC_SHA256_Init(&ctx);
    unsigned char buf[1 << 16];
    for (std::size_t n; (n = std::fread(buf, 1, sizeof(buf), f)) > 0;) CC_SHA256_Update(&ctx, buf, static_cast<CC_LONG>(n));
    std::fclose(f);
    unsigned char md[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256_Final(md, &ctx);
    char hex[2 * CC_SHA256_DIGEST_LENGTH + 1];
    for (int i = 0; i < CC_SHA256_DIGEST_LENGTH; ++i) std::snprintf(hex + 2 * i, 3, "%02x", md[i]);
    return hex;
#else
    return "unavailable";
#endif
}

std::string json(const std::string& s) {
    std::string out = "\"";
    for (char c : s) {
        if (c == '"' || c == '\\') out += '\\';
        out += c;
    }
    return out + "\"";
}

std::string mods() {
    std::string out = "[";
    bool first = true;
    for (int i = 0; i < static_cast<int>(Mods::Mod::Count); ++i) {
        const auto mod = static_cast<Mods::Mod>(i);
        if (!Mods::enabled(mod)) continue;
        out += (first ? "" : ", ") + json(Mods::name(mod));
        first = false;
    }
    return out + "]";
}

void writeSidecar(bool final) {
    const std::string path = gBase + ".tape.json";
    std::FILE* f = std::fopen((path + ".tmp").c_str(), "w");
    if (!f) return;
    const auto stats = Input::KpadRecord::stats();
    static const std::string sha = exeSha256();  // computed when the tape opens (seam()), never in play
    std::fprintf(f, "{\n  \"schema\": 1,\n  \"complete\": %s,\n  \"route_csv\": %s,\n  \"kpd\": %s,\n  \"actions\": %s,\n",
                 final ? "true" : "false", json(gRoute).c_str(), json(gBase + ".kpd").c_str(), json(gBase + ".actions").c_str());
    std::fprintf(f, "  \"seams\": %llu,\n  \"exe_sha256\": %s,\n  \"source\": %s,\n  \"mods\": %s,\n",
                 static_cast<unsigned long long>(gRow), json(sha).c_str(),
                 json(std::getenv("PETARI_TAPE_SOURCE") ? std::getenv("PETARI_TAPE_SOURCE") : "").c_str(), mods().c_str());
    std::fprintf(f, "  \"kpd_reads\": %llu,\n  \"kpd_samples\": %llu,\n  \"unencodable_samples\": %llu,\n  \"lossy_samples\": %llu,\n  \"mod_lines\": %llu,\n",
                 static_cast<unsigned long long>(stats.reads), static_cast<unsigned long long>(stats.samples),
                 static_cast<unsigned long long>(stats.unencodable), static_cast<unsigned long long>(stats.lossy),
                 static_cast<unsigned long long>(stats.modLines));
    std::fputs("  \"ready_seams\": [", f);
    for (std::size_t i = 0; i < gReadys.size(); ++i) {
        const auto& r = gReadys[i];
        std::fprintf(f, "%s\n    {\"row\": %llu, \"stage\": %s, \"scenario\": %d, \"rng_seed\": %u}", i ? "," : "",
                     static_cast<unsigned long long>(r.row), json(r.stage).c_str(), r.scenario, r.seed);
    }
    std::fputs("\n  ],\n  \"segments\": [", f);
    for (std::size_t i = 0; i < gSegments.size(); ++i) {
        const auto& g = gSegments[i];
        const std::uint64_t end = g.open ? (gRow > g.start ? gRow - 1 : g.start) : g.end;
        std::fprintf(f, "%s\n    {\"start_row\": %llu, \"end_row\": %llu, \"frames\": %llu, \"stage\": %s, \"scenario\": %d, \"rng_seed\": %u}", i ? "," : "",
                     static_cast<unsigned long long>(g.start), static_cast<unsigned long long>(end), static_cast<unsigned long long>(end - g.start),
                     json(g.stage).c_str(), g.scenario, g.seed);
    }
    std::fputs("\n  ],\n  \"replay_env\": {", f);
    {
        const char* rts = std::getenv("PETARI_TEST_RETURN_TO_SHIP");  // a test hook that changes the scene: the replay must fire it too
        std::fprintf(f, "%s", rts && *rts ? (std::string("\"PETARI_TEST_RETURN_TO_SHIP\": ") + json(rts)).c_str() : "");
    }
    std::fputs("},\n  \"play_ready_seams\": [", f);
    for (std::size_t i = 0; i < gPlayReadys.size(); ++i) {
        const auto& r = gPlayReadys[i];
        std::fprintf(f, "%s\n    {\"row\": %llu, \"stage\": %s, \"scenario\": %d, \"rng_seed\": %u}", i ? "," : "",
                     static_cast<unsigned long long>(r.row), json(r.stage).c_str(), r.scenario, r.seed);
    }
    std::fputs("\n  ]\n}\n", f);
    std::fclose(f);
    std::rename((path + ".tmp").c_str(), path.c_str());
}

void finish() {
    if (!gOpen) return;
    Input::KpadRecord::close();
    writeSidecar(true);
    gOpen = false;
}

void init() {
    gInit = true;
    const char* dir = std::getenv("PETARI_TAPE_RECORD");
    const char* route = std::getenv("PETARI_ROUTE_RECORD");
    if (dir && *dir) {
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        gRoute = std::string(dir) + "/route.csv";
        gBackground = true;
    } else if (route && *route) {
        gRoute = route;
    } else {
        return;
    }
    gBase = gRoute.size() > 4 && gRoute.compare(gRoute.size() - 4, 4, ".csv") == 0 ? gRoute.substr(0, gRoute.size() - 4) : gRoute;
}

}  // namespace

const char* routePath() {
    if (!gInit) init();
    return gRoute.empty() ? nullptr : gRoute.c_str();
}

bool background() {
    if (!gInit) init();
    return gBackground;
}

void seam(const Seam& s) {
    if (!gInit) init();
    if (gRoute.empty()) return;
    if (!gOpen) {
        std::string error;
        if (!Input::KpadRecord::open(gBase, &error)) {
            std::fprintf(stderr, "PETARI TAPE RECORD: %s\n", error.c_str());
            gRoute.clear();
            return;
        }
        gOpen = true;
        writeSidecar(false);  // hashes the executable now (a startup cost), not at the first checkpoint during play
        std::atexit(finish);
        std::fprintf(stderr, "PETARI TAPE RECORD: writing %s.kpd / .actions / .tape.json\n", gBase.c_str());
    }
    {   // Segments: one per scene visit that reached play-ready (see Segment above).
        const bool isGame = std::strcmp(s.scene, "Game") == 0;
        const std::string visit = std::string(s.scene) + "\n" + s.stage + "\n" + std::to_string(s.scenario);
        const bool newVisit = s.initEnd && visit != gSceneKey;
        if ((!s.initEnd || newVisit) && !gSegments.empty() && gSegments.back().open) {
            gSegments.back().open = false;
            gSegments.back().end = gRow > 0 ? gRow - 1 : 0;
            writeSidecar(false);
        }
        if (newVisit) gVisitHasSegment = false;
        if (isGame && s.initEnd && s.playReady && s.seedValid && !gVisitHasSegment) {
            gSegments.push_back({gRow, gRow, s.stage, s.scenario, s.seed, true});
            gVisitHasSegment = true;
            writeSidecar(false);
        }
    }
    // A scene visit's first Game/End seam is a ready seam (the one petari_headless --sync-stage selects).
    const std::string key = std::string(s.scene) + "\n" + s.stage + "\n" + std::to_string(s.scenario);
    if (s.initEnd && std::strcmp(s.scene, "Game") == 0 && s.seedValid && key != gSceneKey) {
        gReadys.push_back({gRow, s.stage, s.scenario, s.seed});
        writeSidecar(false);
    }
    // Play-ready (the game's own test, native/app/play_ready.cpp) is the replay anchor: the opening flyover's length depends on
    // asynchronous loading, so the Game/End seam is not the same game state across runs, but the first controllable seam is.
    if (s.playReady && !gWasPlayReady && s.seedValid) {
        gPlayReadys.push_back({gRow, s.stage, s.scenario, s.seed});
        writeSidecar(false);
    }
    gWasPlayReady = s.playReady;
    gSceneKey = s.initEnd ? key : std::string();
    ++gRow;
    Input::KpadRecord::seam();
    // Every 60 seams (when the route CSV flushes too) the files are made valid for the loops so far, so a session that is
    // killed instead of quit still leaves a replayable tape (the sidecar then says complete=false and where it stops).
    if (gRow % 60 == 0) {
        Input::KpadRecord::checkpoint();
        if (gRow % 300 == 0) writeSidecar(false);  // the sidecar may lag the tape (it is valid to the loops it names); it is small but a file replace
    }
}

}  // namespace PetariNative::App::TapeRecord
