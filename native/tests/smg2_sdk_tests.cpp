#include <revolution/kmpls.h>
#include <revolution/wbc.h>
#include <petari/input.hpp>
#include <petari/input_remap.hpp>

extern "C" void __OSThreadInit(void);

#include <csignal>
#include <cmath>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>

namespace {
int checks = 0;
void check(bool condition, const char* message) {
    ++checks;
    if (!condition) {
        std::fprintf(stderr, "%s\n", message);
        std::exit(1);
    }
}

void expectAbort(void (*call)()) {
    const pid_t child = fork();
    check(child >= 0, "fork failed");
    if (child == 0) {
        std::freopen("/dev/null", "w", stderr);
        call();
        _exit(0);
    }
    int status = 0;
    check(waitpid(child, &status, 0) == child, "waitpid failed");
    check(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT, "unsupported SDK call must abort loudly");
}

// One initialized SDK instance for all scenarios: SMG2's KPADInitEx is
// deliberately one-shot. Pump the real WPAD/KPAD pipeline at 200/60 Hz.
struct ControlsRig {
    KPADStatus reports[2][120]{};
    std::vector<Vec> core[2], extension;
    int frameCount = 0, reportCount = 0;

    void frame() {
        namespace In = PetariNative::Input;
        const int next = (frameCount + 1) * 10 / 3;
        In::pumpReports(next - reportCount);
        reportCount = next;
        ++frameCount;
        for (s32 chan = 0; chan < 2; ++chan) {
            const int count = KPADRead(chan, reports[chan], 120);
            for (int i = count - 1; i >= 0; --i) {
                core[chan].push_back(reports[chan][i].acc);
                if (chan == 0) extension.push_back(reports[chan][i].ex_status.fs.acc);
            }
        }
    }
    void frames(int count) { for (int i = 0; i < count; ++i) frame(); }
    static bool distanceSwing(const std::vector<Vec>& history, float threshold) {
        if (history.size() < 21) return false;
        const auto& current = history.back();
        const auto& past = history[history.size() - 21];
        const float x = current.x - past.x, y = current.y - past.y, z = current.z - past.z;
        return std::sqrt(x * x + y * y + z * z) >= threshold;
    }
};

void testControlsThroughKpad() {
    namespace In = PetariNative::Input;
    ControlsRig rig;
    In::setViewport(In::Viewport::letterbox(1280, 720, 16.0f / 9.0f));
    KPADSetAccParam(0, 0.0f, 0.15f);  // WPadAcceleration's real filter settings
    KPADSetAccParam(1, 0.0f, 0.15f);
    rig.frames(60);
    // The game compares 20 acceleration reports, with 2g for the Nunchuk
    // and 1g for the Remote. Test the SDK-filtered histories, not raw model
    // floats. This checks that contract; it does not link GameSystem/WPad.
    for (auto button : {In::KeyCode(0), In::Key::C, In::Key::LeftShift}) {
        for (int phase = 0; phase < 3; ++phase) {
            In::focusChanged(false);
            In::focusChanged(true);
            rig.frames(60 + phase);
            if (button != 0) In::keyEvent(button, true, false);
            rig.frame();
            In::keyEvent(In::Key::H, true, false);
            bool previous = false, previousCore = false;
            int edges = 0, coreEdges = 0, first = -1;
            for (int i = 0; i < 90; ++i) {
                rig.frame();
                const bool swing = ControlsRig::distanceSwing(rig.extension, 2.0f);
                if (swing && !previous) { ++edges; if (first < 0) first = i; }
                previous = swing;
                const bool core = ControlsRig::distanceSwing(rig.core[0], 1.0f);
                coreEdges += core && !previousCore;
                previousCore = core;
            }
            check(edges == 1 && coreEdges == 0, "H must cross the SDK-filtered Nunchuk threshold once, without shaking the Remote");
            check(button == 0 || first >= 10, "C/Z exclusion must finish before a Nunchuk swing reaches the game");
        }
    }
    In::focusChanged(false);
    In::focusChanged(true);
    rig.frames(60);
    check(WPADProbe(1, nullptr) == WPAD_ERR_NO_CONTROLLER, "co-star must initially be disconnected");
    In::keyEvent(In::Key::F2, true, false);
    rig.frame();
    In::keyEvent(In::Key::F2, false, false);
    rig.frames(40);
    check(WPADProbe(1, nullptr) == WPAD_ERR_NONE, "F2 must connect WPAD channel 1");
    In::mouseMoved(640, 360);
    rig.frames(20);
    check(rig.reports[0][0].dpd_valid_fg == 2 && rig.reports[1][0].dpd_valid_fg == 2,
          "both KPAD pointers must see the shared mouse");
    check(std::fabs(rig.reports[0][0].pos.x - rig.reports[1][0].pos.x) < 0.01f &&
              std::fabs(rig.reports[0][0].pos.y - rig.reports[1][0].pos.y) < 0.01f,
          "shared mouse coordinates must agree after per-channel KPAD calibration");
    In::keyEvent(In::Key::U, true, false);
    rig.frame();
    check((rig.reports[1][0].hold & WPAD_BUTTON_A) && !(rig.reports[0][0].hold & WPAD_BUTTON_A),
          "U must be channel 1 A only");
    In::keyEvent(In::Key::Y, true, false);
    rig.frame();
    check((rig.reports[1][0].hold & WPAD_BUTTON_B) && !(rig.reports[0][0].hold & WPAD_BUTTON_B),
          "Y must be channel 1 B only");
    In::keyEvent(In::Key::U, false, false);
    In::keyEvent(In::Key::Y, false, false);
    rig.frames(20);
    In::keyEvent(In::Key::Space, true, false);
    rig.frame();
    check((rig.reports[0][0].hold & WPAD_BUTTON_A) && !(rig.reports[1][0].hold & WPAD_BUTTON_A),
          "Space must remain channel 0 A only");
    In::keyEvent(In::Key::Space, false, false);
    In::keyEvent(In::Key::N, true, false);
    int edges = 0;
    bool previous = false;
    for (int i = 0; i < 60; ++i) {
        rig.frame();
        const bool swing = ControlsRig::distanceSwing(rig.core[1], 1.0f);
        edges += swing && !previous;
        previous = swing;
    }
    check(edges == 1, "N must reach the second Remote through SDK filtering once");
    In::focusChanged(false);
    rig.frames(20);
    check(!(rig.reports[1][0].hold & (WPAD_BUTTON_A | WPAD_BUTTON_B)) && !rig.reports[1][0].dpd_valid_fg,
          "focus loss must release second-channel buttons and hide its pointer");
    In::focusChanged(true);
    In::keyEvent(In::Key::F2, true, false);
    rig.frame();
    In::keyEvent(In::Key::F2, false, false);
    check(WPADProbe(1, nullptr) == WPAD_ERR_NO_CONTROLLER, "F2 must disconnect the second Remote");
    auto remapped = In::Bindings::defaults();
    std::string error;
    check(remapped.parse("CoStarToggle=Key:M\nCoStarA=Mouse:X1\n", &error), "live co-star remaps must parse");
    In::setBindings(remapped);
    In::keyEvent(In::Key::M, true, false);
    rig.frame();
    In::keyEvent(In::Key::M, false, false);
    rig.frames(40);
    check(WPADProbe(1, nullptr) == WPAD_ERR_NONE, "remapped toggle must reconnect the second Remote");
    In::keyEvent(In::Key::U, true, false);
    rig.frame();
    check(!(rig.reports[1][0].hold & WPAD_BUTTON_A), "old co-star A key must stop working after live remap");
    In::mouseButtonEvent(In::MouseButton::X1, true);
    rig.frame();
    check((rig.reports[1][0].hold & WPAD_BUTTON_A) && !(rig.reports[0][0].hold & WPAD_BUTTON_A),
          "remapped mouse button must reach channel 1 only");
    In::mouseButtonEvent(In::MouseButton::X1, false);
}

}  // namespace

int main() {
    check(!KMPLSIsInit(), "MotionPlus must not be advertised as initialized");
    for (s32 chan = 0; chan < WPAD_MAX_CONTROLLERS; ++chan) {
        check(WPADiGetMplsStatus(chan) == 0, "native controller must report MotionPlus disabled");
    }
    expectAbort([] { KMPLSRead(0, nullptr, 0, 1, nullptr, 0.0); });
    expectAbort([] { WBCRead(nullptr, nullptr, 4); });
    expectAbort([] { WPADiGetMplsStatus(WPAD_MAX_CONTROLLERS); });
    __OSThreadInit();
    namespace In = PetariNative::Input;
    In::resetForTesting();
    In::setClock(In::Clock::Manual);
    KPADInit();
    In::pumpReports(80);
    u32 type = WPAD_DEV_NOT_FOUND;
    check(WPADProbe(0, &type) == WPAD_ERR_NONE && type == WPAD_DEV_FREESTYLE,
          "SMG2 KPAD must connect the native Nunchuk without MotionPlus/Balance Board calls");
    KPADStatus reports[16]{};
    // SMG2 applies its initial reset on the first read, consuming the old
    // ring contents. Subsequent reports must reach the initialized reader.
    KPADRead(0, reports, 16);
    In::pumpReports(8);
    check(KPADRead(0, reports, 16) > 0, "SMG2 KPAD must return native reports");
    testControlsThroughKpad();
    In::resetForTesting();
    std::printf("SMG2 SDK: %d startup, controls, co-star and unsupported-call checks passed.\n", checks);
    return 0;
}
