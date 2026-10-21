#include <petari/platform/virtual_time.hpp>
#include "../app/headless/reload_policy.hpp"
#include <petari/platform/os_host.hpp>
#include <petari/platform/gx_sync.hpp>
#include <revolution/os.h>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>

extern "C" void __OSThreadInit(void);
namespace OS = PetariNative::Platform::OS;
namespace {
std::vector<unsigned> events;
std::atomic<bool> loaderDone{false};
std::atomic<bool> gpDone{false};
void check(bool value, const char* message) {
    if (!value) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
}
void* pollingLoader(void* priority) {
    petari_os_preemption_point();
    events.push_back(static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(priority)));
    return nullptr;
}
void* loader(void*) {
    events.push_back(2);
    loaderDone = true;
    return nullptr;
}
}
int main() {
    OS::enableVirtualTime(0);
    __OSThreadInit();
    PetariNative::Headless::ReloadPolicy policy;
    check(!policy.observe(false, false), "bootstrap leaves ordering disabled");
    check(!policy.observe(false, true), "pre-target file-select reload leaves ordering disabled");
    check(!policy.observe(true, false), "initial ready scene retains legacy timing");
    check(!policy.observe(true, false), "ordinary ready gameplay retains legacy timing");
    check(policy.observe(true, true), "first post-ready reload requests activation");
    check(policy.observe(false, false), "activation persists across subsequent scenes");
    check(!OS::virtualAsyncActive(), "virtual clock alone does not enable ordered dispatch");
    const auto legacyFirst = OS::queueVirtualAsync();
    const auto legacySecond = OS::queueVirtualAsync();
    std::atomic<bool> legacyDone{false}, releaseLegacy{false};
    std::thread legacyOutOfOrder([&] {
        check(OS::waitVirtualAsync(legacySecond), "legacy second request can arrive first while CPU runs");
        OS::finishVirtualAsync(legacySecond);
    });
    legacyOutOfOrder.join();
    std::thread inFlight([&] {
        check(OS::waitVirtualAsync(legacyFirst), "legacy request remains tracked until completion");
        while (!releaseLegacy) std::this_thread::yield();
        legacyDone = true;
        OS::finishVirtualAsync(legacyFirst);
    });
    petari_os_begin_host_blocking();
    std::thread activation([] { OS::activateVirtualAsync(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    check(!OS::virtualAsyncActive(), "activation fences in-flight legacy work");
    releaseLegacy = true;
    inFlight.join(); activation.join();
    check(legacyDone && OS::virtualAsyncActive(), "ordering starts only after legacy work drains");
    petari_os_end_host_blocking();
    for (const unsigned delay : {0u, 10u}) {
        events.clear(); loaderDone = false;
        OSThread thread{}; alignas(32) unsigned char stack[32768];
        check(OSCreateThread(&thread, loader, nullptr, stack + sizeof(stack), sizeof(stack), 17, 0), "create loader");
        const auto first = OS::queueVirtualAsync();
        const auto second = OS::queueVirtualAsync();
        std::atomic<bool> secondReady{false}, delivered{false};
        std::thread b([&] {
            secondReady = true;
            check(OS::waitVirtualAsync(second), "second ticket exists");
            check(loaderDone, "runnable loader settles before next completion");
            events.push_back(3);
            OS::finishVirtualAsync(second);
        });
        std::thread a([&] {
            check(OS::waitVirtualAsync(first), "first ticket exists");
            check(!OS::cancelVirtualAsync(first), "claimed I/O cannot be cancelled underneath its worker");
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            events.push_back(1); delivered = true;
            OSResumeThread(&thread);
            OS::finishVirtualAsync(first);
        });
        while (!secondReady) std::this_thread::yield();
        check(!delivered, "host I/O cannot deliver while emulated CPU runs");
        const auto before = OS::virtualSystemTime();
        petari_os_begin_host_blocking();
        OS::waitVirtualWork();
        petari_os_end_host_blocking();
        a.join(); b.join();
        check(events == std::vector<unsigned>({1, 2, 3}), "ordered I/O and loader execution independent of host delay");
        check(OS::virtualSystemTime() == before, "host I/O latency does not advance virtual ticks");
        check(OSJoinThread(&thread, nullptr), "join loader");
    }
    const auto polling = OS::queueVirtualAsync();
    std::atomic<bool> done{false};
    std::thread completion([&] {
        check(OS::waitVirtualAsync(polling), "polling ticket exists");
        done = true;
        OS::finishVirtualAsync(polling);
    });
    petari_os_preemption_point();
    check(done, "explicit loader poll services ordered I/O without advancing a frame");
    completion.join();
    events.clear();
    OSThread high{}, low{};
    alignas(32) unsigned char highStack[32768], lowStack[32768];
    check(OSCreateThread(&high, pollingLoader, reinterpret_cast<void*>(17), highStack + sizeof(highStack), sizeof(highStack), 17, 0), "create high-priority poller");
    check(OSCreateThread(&low, pollingLoader, reinterpret_cast<void*>(18), lowStack + sizeof(lowStack), sizeof(lowStack), 18, 0), "create low-priority poller");
    const auto multiple = OS::queueVirtualAsync();
    OSResumeThread(&low);
    OSResumeThread(&high);
    std::thread wake([&] {
        check(OS::waitVirtualAsync(multiple), "multiple-poller ticket exists");
        OS::finishVirtualAsync(multiple);
    });
    petari_os_preemption_point();
    petari_os_begin_host_blocking();
    OS::waitVirtualWork();
    petari_os_end_host_blocking();
    wake.join();
    check(events == std::vector<unsigned>({17, 18}), "pollers rejoin atomically in OS priority order");
    check(OSJoinThread(&high, nullptr) && OSJoinThread(&low, nullptr), "join polling loaders");
    // Force the old early-delivery path: the host GP thread has time to
    // deliver while main still owns the emulated CPU. Virtual delivery must
    // instead wait for the SDK sleep and let the lower-priority loader run.
    namespace GP = PetariNative::Platform::GXSync;
    loaderDone = false;
    OSThread sceneDestroy{}; alignas(32) unsigned char sceneStack[32768];
    check(OSCreateThread(&sceneDestroy, loader, nullptr, sceneStack + sizeof(sceneStack), sizeof(sceneStack), 17, 0), "create scene-destroy worker");
    OSResumeThread(&sceneDestroy);
    GP::setDrawDoneCallback([] { gpDone = true; });
    const auto draw = GP::noteDrawDoneIssued();
    GP::reportDrawDone(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    check(!gpDone, "GP completion cannot arrive early while main owns CPU");
    GP::waitDrawDone(draw);
    check(loaderDone, "scene worker runs deterministically before GP wakes main");
    check(OSJoinThread(&sceneDestroy, nullptr), "join scene-destroy worker");
    GP::shutdown();
    const auto cancelled = OS::queueVirtualAsync();
    std::thread cancelledWorker([&] {
        check(!OS::waitVirtualAsync(cancelled), "shutdown releases a queued worker without running its job");
    });
    check(OS::cancelVirtualAsync(cancelled), "cancel not-yet-started I/O without needing a frame");
    cancelledWorker.join();
    std::puts("legacy bootstrap, reload activation, virtual async ordering and loader fence checks passed");
}
