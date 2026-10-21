// One-key playtest capture: see playtest_capture.hpp.
#include "playtest_capture.hpp"
#include "stall_log.hpp"
#include "petari/input.hpp"

#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"

#include <petari/screenshot.hpp>

#undef gx  // the SDK's GX headers define gx as __GXData; this file uses the route row's gravity field

#include <array>
#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <sys/stat.h>

namespace PetariNative::App::PlaytestCapture {
namespace {

constexpr int kHistory = 300;
constexpr std::size_t kRing = StallLog::kRing;

struct Slot {
    RouteRecordFrame row;
    char stage[64] = {};
};

std::array<Slot, kHistory> gRing;
int gNext = 0;
int gCount = 0;
unsigned long gFrame = 0;

const char* captureRoot() {
    static const char* root = std::getenv("PETARI_CAPTURE_DIR");
    return root != nullptr && root[0] != '\0' ? root : nullptr;
}

bool makeDirs(const std::string& path) {
    for (std::size_t i = 1; i <= path.size(); ++i) {
        if (i == path.size() || path[i] == '/') {
            const std::string part = path.substr(0, i);
            if (mkdir(part.c_str(), 0755) != 0 && errno != EEXIST) return false;
        }
    }
    return true;
}

void writeState(const std::string& dir, const RouteRecordFrame& row) {
    std::FILE* f = std::fopen((dir + "/state.txt").c_str(), "w");
    if (f == nullptr) return;
    const GameSystem* system = SingletonHolder< GameSystem >::get();
    const GameSystemSceneController* controller = system != nullptr ? system->mSceneController : nullptr;
    const char* scene = controller != nullptr ? controller->mCurrSceneControlInfo.mScene : "";
    const char* mods = std::getenv("PETARI_MODS");
    const char* log = std::getenv("PETARI_CAPTURE_LOG");
    std::fprintf(f, "game frame (since the app started): %lu\n", gFrame);
    std::fprintf(f, "scene %s, stage %s, scenario %d, ground zone %d\n", scene, row.stage != nullptr ? row.stage : "", row.scenario, row.zone);
    std::fprintf(f, "mario valid %d: position %.3f %.3f %.3f, grounded %d, dead %d\n", row.valid, row.x, row.y, row.z, row.grounded, row.dead);
    if (row.valid) {
        const TVec3f* v = MR::getPlayerVelocity();
        if (v != nullptr) std::fprintf(f, "velocity %.4f %.4f %.4f\n", v->x, v->y, v->z);
    }
    std::fprintf(f, "gravity %.5f %.5f %.5f; camera forward %.5f %.5f %.5f (yaw %.4f rad)\n", row.gx, row.gy, row.gz, row.cx, row.cy, row.cz, row.yaw);
    std::fprintf(f, "power stars %d\n", row.powerStars);
    const Input::BoundState& i = row.input;
    std::fprintf(f, "input now: move %.3f %.3f, jump %d, spin %d, crouch %d\n", i.moveX, i.moveY, i.jump, i.spin, i.crouch);
    std::fprintf(f, "shoot %d; host pointer valid %d, position %.6f %.6f (x right, y down; screen -1..1)\n", i.shoot, i.pointerValid, i.pointerX, i.pointerY);
    std::fprintf(f, "mods: %s\n", mods != nullptr ? mods : "(mods.txt)");
    std::fprintf(f, "controller layout %s; Mario in play %d (Pro B: %s)\n", PetariNative::Input::padLayoutName(PetariNative::Input::padLayout()),
                 PetariNative::Input::playControlActive(), PetariNative::Input::playControlActive() ? "jump" : "back");
    std::fprintf(f, "app log: %s\n", log != nullptr ? log : "(not set)");
    char stalls[kRing * 120 + 160];
    StallLog::formatCapture(StallLog::gLog, kHistory, stalls, sizeof(stalls));
    std::fputs(stalls, f);
    std::fprintf(f, "route.csv: the last %d frames before the press, oldest first\n", gCount);
    std::fprintf(f, "note: (write what felt off here)\n");
    std::fclose(f);
}

void writeCapture(const char* root) {
    char stamp[32];
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_r(&now, &local);
    std::strftime(stamp, sizeof(stamp), "%Y%m%d-%H%M%S", &local);
    std::string dir = std::string(root) + "/" + stamp;
    for (int n = 2; ; ++n) {
        struct stat st {};
        if (stat(dir.c_str(), &st) != 0) break;
        dir = std::string(root) + "/" + stamp + "-" + std::to_string(n);
    }
    if (!makeDirs(dir)) {
        std::fprintf(stderr, "[capture] cannot create %s: %s\n", dir.c_str(), std::strerror(errno));
        return;
    }
    {
        RouteRecordWriter writer;
        if (writer.open((dir + "/route.csv").c_str(), false)) {
            for (int k = 0; k < gCount; ++k) {
                writer.write(gRing[(gNext - gCount + k + kHistory) % kHistory].row);
            }
        }
    }
    const RouteRecordFrame& last = gRing[(gNext - 1 + kHistory) % kHistory].row;
    writeState(dir, last);
    Screenshot::request((dir + "/screen.png").c_str());
    std::fprintf(stderr, "[capture] saved %s (%d frames)\n", dir.c_str(), gCount);
    std::fflush(stderr);
}

}  // namespace

bool enabled(bool background) {
    return !background && captureRoot() != nullptr;
}

void frame(const RouteRecordFrame& row) {
    ++gFrame;
    // Test hook: PETARI_CAPTURE_TEST_FRAME=<n> presses the capture key at recorder frame n (no window input needed).
    static const unsigned long testFrame = [] {
        const char* value = std::getenv("PETARI_CAPTURE_TEST_FRAME");
        return value != nullptr ? std::strtoul(value, nullptr, 10) : 0ul;
    }();
    if (testFrame != 0 && gFrame == testFrame) request();
    Slot& slot = gRing[gNext];
    std::snprintf(slot.stage, sizeof(slot.stage), "%s", row.stage != nullptr ? row.stage : "");
    slot.row = row;
    slot.row.stage = slot.stage;
    gNext = (gNext + 1) % kHistory;
    if (gCount < kHistory) ++gCount;
    if (gRequests.exchange(0, std::memory_order_relaxed) > 0) {
        if (const char* root = captureRoot()) writeCapture(root);
    }
}

}  // namespace PetariNative::App::PlaytestCapture
