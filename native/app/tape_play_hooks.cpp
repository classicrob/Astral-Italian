// The process-wide side of tape replay: the KPAD / mod hooks, the controller-connection feeder thread and the report files.
#include "tape_play.hpp"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include <thread>

#include <petari/kpad_replay.hpp>

namespace PetariNative::App::TapePlay {
namespace H = PetariNative::Headless;
// --- process-wide glue ---
namespace {
std::unique_ptr<Session> gSession;
std::filesystem::path gTapeDir;
bool gReported = false;

std::int32_t hookRead(std::int32_t channel, KPADStatus* buffer, std::uint32_t capacity, std::int32_t* error, void*) {
    return gSession->read(channel, capacity, buffer, error);
}
bool hookPress(int mod, void*) { return gSession->press(mod); }
bool hookHeld(int mod, void*) { return gSession->held(mod); }

// The recorded controller connection (probe state and device type per channel) is applied from a host thread, as the headless
// driver's worker does each frame (replayConnection needs a non-game thread); the same thread completes pending WPAD commands
// every 5 ms (serviceReplayCommands). The game thread publishes the connection of the loop about to run at every seam.
struct Feeder {
    std::mutex mutex;
    std::int32_t error[4] = {-2, -2, -2, -2};
    std::uint32_t device[4] = {253, 253, 253, 253};
    std::atomic<bool> stop{false};
    std::thread thread;
    void publish() {
        std::lock_guard<std::mutex> lock(mutex);
        for (std::uint32_t ch = 0; ch < 4; ++ch)
            if (const auto* record = gSession->connection(ch)) { error[ch] = record->probeError; device[ch] = record->device; }
    }
    void run() {
        while (!stop.load()) {
            std::int32_t e[4];
            std::uint32_t d[4];
            { std::lock_guard<std::mutex> lock(mutex); for (int i = 0; i < 4; ++i) { e[i] = error[i]; d[i] = device[i]; } }
            for (std::uint32_t ch = 0; ch < 4; ++ch) Input::replayConnection(static_cast<std::int32_t>(ch), e[ch], d[ch]);
            Input::serviceReplayCommands();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
};
Feeder* gFeeder = nullptr;
void stopFeeder() {
    if (!gFeeder) return;
    gFeeder->stop.store(true);
    if (gFeeder->thread.joinable()) gFeeder->thread.join();
}

std::string stamp() {
    char text[32];
    const std::time_t now = std::time(nullptr);
    std::strftime(text, sizeof(text), "%Y%m%d-%H%M%S", std::localtime(&now));
    return text;
}

void writeReport() {
    if (gReported || !gSession) return;
    gReported = true;
    std::fputs(gSession->reportText().c_str(), stderr);
    // Next to a copy of the tape (the tape itself is never modified): <out>/{route.*, divergence.json, divergence.txt}.
    const char* configured = std::getenv("PETARI_TAPE_PLAY_OUT");
    std::filesystem::path out = configured && *configured ? std::filesystem::path(configured)
        : gTapeDir.parent_path() / (gTapeDir.filename().string() + "-replay-" + stamp());
    std::error_code ec;
    std::filesystem::create_directories(out, ec);
    for (const char* name : {"route.tape.json", "route.csv", "route.kpd", "route.actions"})
        std::filesystem::copy_file(gTapeDir / name, out / name, std::filesystem::copy_options::overwrite_existing, ec);
    std::ofstream(out / "divergence.json") << gSession->reportJson();
    std::ofstream(out / "divergence.txt") << gSession->reportText();
    std::fprintf(stderr, "PETARI TAPE PLAY: report written to %s\n", out.string().c_str());
}
}  // namespace

bool configure(const std::string& dir, std::string* error) {
    try {
        gTapeDir = std::filesystem::absolute(dir);
        gSession = std::make_unique<Session>(gTapeDir);
    } catch (const std::exception& e) {
        gSession.reset();
        if (error) *error = e.what();
        return false;
    }
    Input::setKpadReplayHooks({hookRead, nullptr});
    Input::setModReplayHooks({hookPress, hookHeld, nullptr});
    std::atexit([] { writeReport(); });  // a window closed early still leaves the report of what was replayed
    gFeeder = new Feeder;
    gFeeder->publish();
    gFeeder->thread = std::thread([] { gFeeder->run(); });
    std::atexit(stopFeeder);
    return true;
}

bool active() { return gSession != nullptr; }
const SegmentMeta* firstSegment() { return gSession ? &gSession->firstSegment() : nullptr; }
const Meta* meta() { return gSession ? &gSession->meta() : nullptr; }

Step seam(const SeamState& state) {
    if (!gSession) return {};
    const auto before = gSession->currentSegment();
    const bool wasDone = gSession->finished();
    const Step step = gSession->seam(state);
    if (!wasDone && gSession->currentSegment() != before)
        std::fprintf(stderr, "PETARI TAPE PLAY: %s\n", gSession->segmentLine(before).c_str());
    if (gFeeder) gFeeder->publish();
    if (step.kind == Step::Kind::Finished || step.kind == Step::Kind::GaveUp) writeReport();
    return step;
}

bool shouldQuit() { return gSession && gSession->finished(); }

}  // namespace PetariNative::App::TapePlay
