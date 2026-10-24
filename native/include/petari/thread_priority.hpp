#pragma once
// Aurora names its main and FIFO processor threads through SDL_SetCurrentThreadPriority(NORMAL), which on macOS gives
// them an explicit SCHED_OTHER policy (requested 37). From then on their QoS no longer applies and nice lowers them,
// while the pipeline compile workers keep their QoS classes (user-initiated 37, utility 20). 5.4 compiles during menus,
// so on a busy or niced host the main game thread was descheduled while it held the emulated CPU: baton preemption
// waits of ~100 ms, audio underruns and 3x slower menus (server thread probe, 2026-10-23; main thread 17 at nice 19,
// below the utility prewarm workers). The render worker already takes the top of SCHED_OTHER the same way
// (present_aurora.cpp). Called once on the thread itself, after Aurora/SDL has set its policy.

#include <pthread.h>
#include <sched.h>

#include <cstdio>

namespace PetariNative::ThreadPriority {

inline void raiseToSchedOtherMax(const char* name) noexcept {
    int policy = 0;
    sched_param param{};
    pthread_getschedparam(pthread_self(), &policy, &param);
    const int before = param.sched_priority;
    param.sched_priority = sched_get_priority_max(SCHED_OTHER);
    const int result = pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
    std::fprintf(stderr, "[thread] %s priority: %d -> %d (%s)\n", name, before, param.sched_priority,
                 result == 0 ? "raised" : "unchanged, failed");
}

}  // namespace PetariNative::ThreadPriority
