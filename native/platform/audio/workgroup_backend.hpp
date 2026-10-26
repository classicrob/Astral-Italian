#pragma once
#include <cstdint>
#include <os/workgroup.h>
#include <mach/thread_policy.h>

namespace PetariNative::Platform::AudioWorkgroup::Backend {
struct Policy {
    thread_extended_policy_data_t extended{};
    thread_time_constraint_policy_data_t realtime{};
    thread_time_constraint_policy_data_t actual{};
    bool wasRealtime = false, saved = false, changed = false;
};
struct Api {
    os_workgroup_interval_t (*create)();
    void (*release)(os_workgroup_interval_t);
    int (*promote)(Policy&);
    int (*restore)(Policy&);
    int (*join)(os_workgroup_t, os_workgroup_join_token_t);
    void (*leave)(os_workgroup_t, os_workgroup_join_token_t);
    int (*start)(os_workgroup_interval_t, std::uint64_t, std::uint64_t);
    int (*finish)(os_workgroup_interval_t);
};
#ifdef PETARI_AUDIO_WORKGROUP_TESTING
// Only before any session; native fixture supplies a complete fake API. Never
// compiled into the app and never mutates a test runner's scheduling policy.
void setApi(const Api&);
void requestForTest(bool);
#endif
}
