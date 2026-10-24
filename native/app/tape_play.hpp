#pragma once
// Tape replay in the GX app's normal window (docs: native/PLAYTEST.md "Replaying your tape"). A tape (PETARI_TAPE_RECORD,
// native/app/tape_record.hpp) is replayed the way petari_headless replays it: each segment starts at its own play-ready seam
// (the headless --sync-play rule), the recorded RNG seed is set there, and the segment's recorded KPAD reads and mod buttons
// are fed; between segments the reads are neutral. PETARI_TAPE_PLAY_SESSION=1 instead boots normally, anchors title and every
// Game/End scene visit (including FileSelect and WorldMap), feeds menu reads too, never warps, and stops at the first divergence.
// Scene construction loops are neutral while waiting for initialize-End; their asynchronous lengths are not replay anchors.
// The feeder is the headless Replay (native/app/headless/replay.hpp).
//
// Session is game-free (unit-tested by native/tests/tape_play_tests.cpp); the glue that reads the game at the frame seam is
// in smoke_game.cpp (recordRouteFrame) and app_main.cpp (--replay-tape / PETARI_TAPE_PLAY).
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "headless/replay.hpp"

namespace PetariNative::App::TapePlay {

struct SegmentMeta {
    std::uint64_t startRow = 0, endRow = 0;
    std::string stage;
    int scenario = 0;
    std::uint32_t seed = 0;
};
struct Meta {
    std::uint64_t seams = 0;
    bool complete = false;
    std::string exeSha256;
    std::vector<std::string> mods;
    std::vector<SegmentMeta> segments;
    std::vector<SegmentMeta> sceneStarts;  // ready_seams: includes FileSelect and the galaxy map
};
// The sidecar (<dir>/route.tape.json) text -> Meta. Throws Headless::ReplayError.
Meta parseSidecar(const std::string& text);

// What the game looked like at one frame seam (the same fields as a route.csv row).
struct SeamState {
    std::string stage;
    int scenario = -1;
    bool valid = false, grounded = false, dead = false;
    float x = 0, y = 0, z = 0;
    bool playReady = false;  // Host::playerInPlay() as of the previous seam's update (read before this seam's update)
    bool initEnd = false;
};
// What the glue must do after a seam.
struct Step {
    enum class Kind { None, Start, Warp, Finished, GaveUp } kind = Kind::None;
    std::uint32_t seed = 0;  // Start: set the game's RNG seed now
    bool restoreSeed = true; // the schema-1 tape has no RNG checkpoint for the initial title row
    std::string stage;       // Warp: take the game to this stage and scenario (a new visit)
    int scenario = 0;
};

struct SegmentReport {
    int index = 0;
    std::string stage;
    int scenario = 0;
    std::uint64_t startRow = 0, frames = 0, rowsCompared = 0;
    bool started = false, completed = false;
    bool diverged = false;
    std::uint64_t firstDivergenceRow = 0;  // segment-relative row k (route row startRow + k)
    double firstDivergenceDist = 0, maxDist = 0;
    std::string firstDivergenceWhy;
    float firstReplay[3] = {0, 0, 0}, firstRecorded[3] = {0, 0, 0};  // positions at the first divergence
    std::string firstReplayStage, firstRecordedStage;
    std::uint64_t reads = 0, pointingReads = 0;
    float recordedPointer[2] = {0, 0}, replayedPointer[2] = {0, 0};
    int pointerValid = 0;
};

// After a segment ended, the game gets this many loops to reach the next segment by itself (the ship's helm -> the world map ->
// the galaxy are the player's untaped menu presses); then the glue jumps there with a direct stage change.
constexpr std::uint64_t kWarpDelaySeams = 300;

class Session {
public:
    enum class Mode { Segments, WholeSession };
    // Loads <dir>/route.tape.json, route.kpd, route.actions (optional) and route.csv. Throws Headless::ReplayError.
    explicit Session(const std::filesystem::path& dir, Mode mode = Mode::Segments);
    const Meta& meta() const { return mMeta; }
    const std::vector<SegmentReport>& report() const { return mReport; }
    const SegmentMeta& firstSegment() const { return mMeta.segments.at(mPlan.front().index); }
    // Once per frame seam, after the game loop ran (post-endFrame), before the next loop's reads.
    Step seam(const SeamState& state);
    // The KPAD read hook (game thread). A replay that cannot be followed (the game read differently than recorded) is given up
    // and reads are neutral from then on; error() says why.
    std::int32_t read(std::int32_t channel, std::uint32_t capacity, void* buffer, std::int32_t* error);
    bool press(int mod);
    bool held(int mod);
    const Headless::Read* connection(std::uint32_t channel) const;
    bool finished() const { return mDone; }
    bool gaveUp() const { return !mError.empty(); }
    const std::string& error() const { return mError; }
    std::string reportText() const;
    std::string segmentLine(std::size_t index) const;  // one report line for a segment
    std::string reportJson() const;
    std::uint64_t seams() const { return mSeams; }
    std::size_t segmentCount() const { return mPlan.size(); }
    std::size_t currentSegment() const { return mSegIndex; }
    bool active() const { return mActive; }
    bool wholeSession() const { return mMode == Mode::WholeSession; }
private:
    struct Planned {
        std::size_t index;
        std::uint64_t start, end, frames, offset;
    };
    struct CsvRow {
        std::string stage;
        int scenario = 0;
        bool valid = false, grounded = false, dead = false;
        float x = 0, y = 0, z = 0;
    };
    void compare(const SeamState& state, std::uint64_t k);
    void inputFailure(const std::string& why, const SeamState& lastObserved);
    Mode mMode;
    SeamState mLastObserved;
    Meta mMeta;
    std::vector<Planned> mPlan;
    std::vector<CsvRow> mRoute;
    std::unique_ptr<Headless::Replay> mReplay;
    std::map<std::pair<std::uint64_t, int>, std::pair<bool, bool>> mActions;  // (tape frame, mod) -> press, held
    std::uint64_t mConsumedFrame = UINT64_MAX;
    unsigned mConsumedMask = 0;
    std::vector<SegmentReport> mReport;
    std::size_t mSegIndex = 0;
    std::uint64_t mSegRow = 0, mSeams = 0, mWaitSeams = 0;
    bool mActive = false, mDone = false, mNeedEdge = false, mWarpSent = false;
    std::string mError;
};

// --- process-wide glue (app_main, frame seam) ---
// Load the tape and install the replay hooks (before WPADInit). On failure *error says why; nothing is installed.
bool configure(const std::string& dir, std::string* error);
bool active();                       // a tape replay was configured
const SegmentMeta* firstSegment();   // legacy stage to launch into; null for whole-session ordinary title boot
const Meta* meta();
// Frame seam; returns what the glue must do (the caller sets the RNG seed / queues the warp). Also writes the report at the end.
Step seam(const SeamState& state);
bool shouldQuit();                   // the replay finished or gave up: the app may close itself
}  // namespace PetariNative::App::TapePlay
