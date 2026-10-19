#include <revolution/kmpls.h>
#include <revolution/wbc.h>
#include <petari/input.hpp>

extern "C" void __OSThreadInit(void);

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>

namespace {
void check(bool condition, const char* message) {
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
    check(KPADRead(0, reports, 16) > 0, "SMG2 KPAD must return native reports");
    In::resetForTesting();
    std::puts("SMG2 KPAD startup, extension absence and unsupported-call checks passed.");
    return 0;
}
