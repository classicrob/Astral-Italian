#include <petari/floating_point.hpp>
// OSAlarm on the host.
//
// Port of src/RVL_SDK/os/OSAlarm.c. The alarm queue and its ordering are the
// SDK's; the decrementer exception is replaced by a timer host thread that
// sleeps until the earliest alarm, then runs its handler as an interrupt
// handler would: interrupts disabled, scheduler disabled, then a reschedule.
// Handlers receive the context of the thread that was running (or of an idle
// context when no OS thread was running).

#include <pthread.h>

#include <chrono>
#include <condition_variable>

#include "os_internal.hpp"
#include "petari/host_allocation.hpp"
#include "petari/platform/virtual_time.hpp"

namespace PetariNative::Platform::OS {
std::chrono::steady_clock::time_point systemTimeToHost(OSTime systemTime);
}

using namespace PetariNative::Platform::OS;

namespace {

struct OSAlarmQueue {
    OSAlarm* head;
    OSAlarm* tail;
} AlarmQueue;

OSContext IdleContext;
bool gTimerStarted;

std::condition_variable& timerCv() {
    static auto* instance = [] {
        PetariNative::HostAllocationScope hostAllocations;  // first use may be on a game thread
        return new std::condition_variable;
    }();
    return *instance;
}

void* timerMain(void*);

// Lock held. Replaces programming the decrementer.
void SetTimer(OSAlarm*) {
    if (virtualTimeEnabled()) {
        if (gTimerStarted) fatal("virtual clock selected after alarm timer startup");
        return;
    }
    if (!gTimerStarted) {
        gTimerStarted = true;
        PetariNative::HostAllocationScope hostAllocations;
        pthread_t handle;
        pthread_attr_t attrs;
        pthread_attr_init(&attrs);
        pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
        if (pthread_create(&handle, &attrs, timerMain, nullptr) != 0) {
            fatal("cannot start the OS alarm timer thread");
        }
        pthread_attr_destroy(&attrs);
    }
    PETARI_GPU_TRACE(InterruptTrace::notify(&interruptMutex(), &timerCv(), nullptr, "alarm-cv", false));
    timerCv().notify_all();
    PETARI_GPU_TRACE(InterruptTrace::notify(&interruptMutex(), &timerCv(), nullptr, "alarm-cv", true));
}

void InsertAlarm(OSAlarm* alarm, OSTime fire, OSAlarmHandler handler, bool reschedule = false) {
    OSAlarm* next;
    OSAlarm* prev;

    if (0 < alarm->period) {
        OSTime time = __OSGetSystemTime();
        fire = alarm->start;
        if (alarm->start < time || (reschedule && alarm->start == time)) {
            fire += alarm->period * ((time - alarm->start) / alarm->period + 1);
        }
    }

    alarm->handler = handler;
    alarm->fire = fire;

    for (next = AlarmQueue.head; next; next = next->next) {
        if (next->fire <= fire) {
            continue;
        }
        alarm->prev = next->prev;
        next->prev = alarm;
        alarm->next = next;
        prev = alarm->prev;
        if (prev != nullptr) {
            prev->next = alarm;
        } else {
            AlarmQueue.head = alarm;
            SetTimer(alarm);
        }
        return;
    }

    alarm->next = nullptr;
    prev = AlarmQueue.tail;
    AlarmQueue.tail = alarm;
    alarm->prev = prev;
    if (prev != nullptr) {
        prev->next = alarm;
    } else {
        AlarmQueue.head = AlarmQueue.tail = alarm;
        SetTimer(alarm);
    }
}

// The decrementer interrupt: runs expired alarms. Interrupt lock held.
void fireExpired() {
    while (AlarmQueue.head != nullptr && AlarmQueue.head->fire <= __OSGetSystemTime()) {
        OSAlarm* alarm = AlarmQueue.head;
        OSAlarm* next = alarm->next;
        AlarmQueue.head = next;
        if (next == nullptr) {
            AlarmQueue.tail = nullptr;
        } else {
            next->prev = nullptr;
        }

        OSAlarmHandler handler = alarm->handler;
        alarm->handler = nullptr;
        if (0 < alarm->period) {
            InsertAlarm(alarm, 0, handler, true);
        }

        OSThread* interrupted = OSGetCurrentThread();
        OSContext* context = interrupted ? &interrupted->context : &IdleContext;
        OSDisableScheduler();
        handler(alarm, context);
        OSEnableScheduler();
        __OSReschedule();
    }
}

void* timerMain(void*) {
    PetariNative::FloatingPointThreadPolicy::apply();
    std::condition_variable& cv = timerCv();
    OSDisableInterrupts();
    std::unique_lock<std::mutex> lock(interruptMutex(), std::adopt_lock);
    while (true) {
        if (AlarmQueue.head == nullptr) {
            setInterruptOwner(false);
            PETARI_GPU_TRACE(PetariNative::GpuTrace::waitBegin("alarm-cond-untimed"));
            PETARI_GPU_TRACE(InterruptTrace::cvReleasePending(&interruptMutex(), &cv, "alarm-untimed-unlock-pending"));
            cv.wait(lock);
            PETARI_GPU_TRACE(InterruptTrace::cvReacquired(&interruptMutex(), &cv, "alarm-wait-return-reacquired"));
            PETARI_GPU_TRACE(PetariNative::GpuTrace::waitEnd());
            setInterruptOwner(true);
            continue;
        }
        const auto deadline = systemTimeToHost(AlarmQueue.head->fire);
        if (std::chrono::steady_clock::now() < deadline) {
            setInterruptOwner(false);
            PETARI_GPU_TRACE(PetariNative::GpuTrace::waitBegin("alarm-cond-until",
                std::chrono::duration_cast<std::chrono::nanoseconds>(deadline.time_since_epoch()).count()));
            PETARI_GPU_TRACE(InterruptTrace::cvReleasePending(&interruptMutex(), &cv, "alarm-timed-unlock-pending"));
            cv.wait_until(lock, deadline);
            PETARI_GPU_TRACE(InterruptTrace::cvReacquired(&interruptMutex(), &cv, "alarm-wait-return-reacquired"));
            PETARI_GPU_TRACE(PetariNative::GpuTrace::waitEnd());
            setInterruptOwner(true);
            continue;
        }
        fireExpired();
        // Deliver scheduling decisions made by handlers, as on interrupt return.
        lock.release();
        OSEnableInterrupts();
        OSDisableInterrupts();
        lock = std::unique_lock<std::mutex>(interruptMutex(), std::adopt_lock);
    }
}

}  // namespace

namespace PetariNative::Platform::OS {

void setVirtualSystemTime(std::int64_t ticks);
void prepareVirtualClock() {
    InterruptGuard guard;
    if (gTimerStarted || AlarmQueue.head) fatal("virtual time must be selected before alarms are installed");
}
void advanceVirtualTimeTo(std::int64_t target) {
    if (boundThread() != nullptr || !virtualTimeEnabled() || target < virtualSystemTime())
        fatal("virtual alarms require a monotonic host interrupt driver");
    InterruptGuard guard;
    unsigned dispatched = 0;
    while (AlarmQueue.head && AlarmQueue.head->fire <= target) {
        const auto fire = AlarmQueue.head->fire;
        setVirtualSystemTime(fire < virtualSystemTime() ? virtualSystemTime() : fire);
        fireExpired();
        if (++dispatched > 1000000) fatal("virtual alarm queue does not make progress");
    }
    setVirtualSystemTime(target);
}

void dumpAlarms(std::FILE* out) {
    const OSTime now = __OSGetSystemTime();
    std::fprintf(out, "[hang] pending alarms (timer thread %s):\n", gTimerStarted ? "started" : "not started");
    int count = 0;
    for (OSAlarm* alarm = AlarmQueue.head; alarm != nullptr && count < 64; alarm = alarm->next, ++count) {
        const double inMs = static_cast<double>(alarm->fire - now) * 1000.0 / OS_TIMER_CLOCK;
        std::fprintf(out, "[hang]   alarm %p: %s in %.1f ms", static_cast<void*>(alarm),
                     describeAddress(reinterpret_cast<std::uint64_t>(alarm->handler)).c_str(), inMs);
        if (alarm->period > 0) {
            std::fprintf(out, ", period %.1f ms", static_cast<double>(alarm->period) * 1000.0 / OS_TIMER_CLOCK);
        }
        std::fputc('\n', out);
    }
    if (count == 0) {
        std::fprintf(out, "[hang]   none\n");
    }
}

}  // namespace PetariNative::Platform::OS

extern "C" {

void OSCreateAlarm(OSAlarm* alarm) {
    alarm->handler = nullptr;
    alarm->tag = 0;
}

void OSSetAlarm(OSAlarm* alarm, OSTime tick, OSAlarmHandler handler) {
    BOOL enabled = OSDisableInterrupts();
    alarm->period = 0;
    InsertAlarm(alarm, __OSGetSystemTime() + tick, handler);
    OSRestoreInterrupts(enabled);
}

void OSSetPeriodicAlarm(OSAlarm* alarm, OSTime start, OSTime period, OSAlarmHandler handler) {
    BOOL enabled = OSDisableInterrupts();
    alarm->period = period;
    alarm->start = __OSTimeToSystemTime(start);
    InsertAlarm(alarm, 0, handler);
    OSRestoreInterrupts(enabled);
}

void OSCancelAlarm(OSAlarm* alarm) {
    BOOL enabled = OSDisableInterrupts();
    if (alarm->handler == nullptr) {
        OSRestoreInterrupts(enabled);
        return;
    }

    OSAlarm* next = alarm->next;
    if (next == nullptr) {
        AlarmQueue.tail = alarm->prev;
    } else {
        next->prev = alarm->prev;
    }
    if (alarm->prev != nullptr) {
        alarm->prev->next = next;
    } else {
        AlarmQueue.head = next;
        if (next != nullptr) {
            SetTimer(next);
        }
    }
    alarm->handler = nullptr;
    OSRestoreInterrupts(enabled);
}

// OSAlarm.c: cancels every queued alarm carrying the tag (0 and -1 are never matched).
void OSCancelAlarms(u32 tag) {
    if (tag == 0 || tag == 0xFFFFFFFF) {
        return;
    }

    BOOL enabled = OSDisableInterrupts();
    OSAlarm* alarm = AlarmQueue.head;
    OSAlarm* next = alarm != nullptr ? alarm->next : nullptr;
    while (alarm != nullptr) {
        if (alarm->tag == tag) {
            OSCancelAlarm(alarm);
        }
        alarm = next;
        next = alarm != nullptr ? alarm->next : nullptr;
    }
    OSRestoreInterrupts(enabled);
}

void OSSetAlarmTag(OSAlarm* alarm, u32 tag) {
    alarm->tag = tag;
}

void OSSetAlarmUserData(OSAlarm* alarm, void* userData) {
    alarm->userData = userData;
}

void* OSGetAlarmUserData(const OSAlarm* alarm) {
    return alarm->userData;
}

}  // extern "C"
