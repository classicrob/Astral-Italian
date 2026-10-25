// OSDisableInterrupts / OSEnableInterrupts / OSRestoreInterrupts on the host.
// See os_internal.hpp for the model.

#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <chrono>

#include "os_internal.hpp"
#include "petari/host_allocation.hpp"

namespace PetariNative::Platform::OS {
namespace {

thread_local bool tDisabled = false;
std::atomic<std::uintptr_t> gOwner{0};
InterruptReleaseTiming gHeldTiming, gLastReleaseTiming; // interruptMutex

void disable() {
    interruptMutex().lock();
    setInterruptOwner(true);
    tDisabled = true;
    onInterruptsDisabled();
}

void enable() {
    onInterruptsEnabling();
    tDisabled = false;
    // QoS overrides detached by changes of hands are ended only once the
    // mutex is released (see os_thread.cpp). The thread-local list keeps its
    // capacity (host allocation), so the swap under the lock does not allocate.
    thread_local std::vector<pthread_override_t> overrides;
    takeDeferredOverrides(overrides);
    setInterruptOwner(false);
    interruptMutex().unlock();
    if (!overrides.empty()) {
        endOverrides(overrides);
    }
}

}  // namespace

std::mutex& interruptMutex() {
    // Never destroyed: interrupt sources may still run during static destruction.
    static std::mutex* instance = [] {
        PetariNative::HostAllocationScope hostAllocations;  // first use may be on a game thread
        return new std::mutex;
    }();
    return *instance;
}

bool interruptsDisabled() {
    return tDisabled;
}

std::uintptr_t interruptOwner() {
    return gOwner.load(std::memory_order_relaxed);
}

void setInterruptOwner(bool held) {
    const auto ns = static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    if (held) {
        thread_local const auto identity = [] {
            InterruptReleaseTiming value;
            value.hostThread = reinterpret_cast<std::uintptr_t>(pthread_self());
            pthread_getname_np(pthread_self(), value.name, sizeof(value.name));
            for (char& c : value.name) if (c == ' ' || c == '\t' || c == '\n') c = '_';
            return value;
        }();
        gHeldTiming = identity;
        gHeldTiming.osThread = reinterpret_cast<std::uintptr_t>(boundThread());
        gHeldTiming.acquiredNs = ns;
    } else {
        gLastReleaseTiming = gHeldTiming;
        gLastReleaseTiming.releasedNs = ns;
    }
    gOwner.store(held ? reinterpret_cast<std::uintptr_t>(pthread_self()) : 0, std::memory_order_relaxed);
}

InterruptReleaseTiming lastInterruptReleaseTiming() {
    return gLastReleaseTiming;
}

void fatal(const char* fmt, ...) {
    std::va_list args;
    va_start(args, fmt);
    std::fputs("OS fatal: ", stderr);
    std::vfprintf(stderr, fmt, args);
    std::fputc('\n', stderr);
    va_end(args);
    std::abort();
}

}  // namespace PetariNative::Platform::OS

using namespace PetariNative::Platform::OS;

extern "C" {

BOOL OSDisableInterrupts(void) {
    if (tDisabled) {
        noteInterruptDisable(true);
        return FALSE;
    }
    disable();
    noteInterruptDisable(false);
    return TRUE;
}

BOOL OSEnableInterrupts(void) {
    if (!tDisabled) {
        return TRUE;
    }
    enable();
    return FALSE;
}

BOOL OSRestoreInterrupts(BOOL level) {
    const BOOL previous = tDisabled ? FALSE : TRUE;
    if (level && tDisabled) {
        enable();
    } else if (!level && !tDisabled) {
        disable();
    }
    return previous;
}

}  // extern "C"
