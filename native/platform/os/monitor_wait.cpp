#include "monitor_wait.hpp"
#include <petari/gpu_trace.hpp>
#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach/thread_policy.h>
#include <sys/event.h>
#include <unistd.h>
#include <pthread.h>

namespace PetariNative::Platform::MonitorWait {
namespace {
std::atomic<Mode> selected{Mode::Legacy};
Timebase timebase;
constexpr std::uint64_t kUser = 1, kTimer = 2;
[[noreturn]] void fatal(const char* site, int error, int cleanup = 0) {
    std::fprintf(stderr, "[baton wait] FATAL mode=%s site=%s error=%d cleanup_error=%d; no fallback or timing admission\n", name(selected.load()), site, error, cleanup);
    std::fflush(stderr); std::abort();
}
class Kqueue {
public:
    std::atomic<int> fd{-1};
    std::atomic<bool> published{false};
    int receipt(kevent64_s change, int allowed = 0) {
        change.flags |= EV_RECEIPT;
        kevent64_s ack{}; timespec immediate{};
        const int count = kevent64(fd.load(std::memory_order_acquire), &change, 1, &ack, 1, 0, &immediate);
        if (count < 0) return errno;
        if (count != 1 || !(ack.flags & EV_ERROR) || ack.ident != change.ident || ack.filter != change.filter) return EPROTO;
        return ack.data == 0 || ack.data == allowed ? 0 : static_cast<int>(ack.data);
    }
    int initialize() {
        const int queue = kqueue();
        if (queue < 0) return errno;
        fd.store(queue, std::memory_order_release);
        if (fcntl(queue, F_SETFD, FD_CLOEXEC) < 0) return errno;
        kevent64_s user{}; user.ident = kUser; user.filter = EVFILT_USER; user.flags = EV_ADD | EV_CLEAR;
        const int error = receipt(user);
        if (!error) published.store(true, std::memory_order_release); // Only AFTER the user filter exists.
        return error;
    }
    int arm(std::uint64_t absolute, std::uint64_t generation, bool critical, std::uint64_t leeway) {
        kevent64_s timer{}; timer.ident = kTimer; timer.filter = EVFILT_TIMER;
        timer.flags = EV_ADD | EV_ENABLE | EV_ONESHOT;
        timer.fflags = NOTE_MACHTIME | NOTE_ABSOLUTE | NOTE_LEEWAY | (critical ? NOTE_CRITICAL : 0);
        timer.data = static_cast<std::int64_t>(absolute); timer.ext[1] = leeway; timer.udata = generation;
        return receipt(timer);
    }
    Batch receive() {
        kevent64_s events[2]{}; Batch batch;
        const int count = kevent64(fd.load(std::memory_order_acquire), nullptr, 0, events, 2, 0, nullptr);
        const int error = count < 0 ? errno : 0;
        PETARI_GPU_TRACE(auto c = PetariNative::GpuTrace::monitorContext; c.detail = static_cast<unsigned>(error);
            PetariNative::GpuTrace::emit("monitor_kevent_return", c, count));
        if (count < 0) { batch.interrupted = error == EINTR; batch.error = batch.interrupted ? 0 : error; return batch; }
        for (int i = 0; i < count; ++i) {
            const auto& event = events[i];
            if (event.flags & EV_ERROR) { batch.error = event.data ? static_cast<int>(event.data) : EPROTO; return batch; }
            if (event.filter == EVFILT_USER && event.ident == kUser) batch.events[batch.count++] = {Kind::Notification, 0};
            else if (event.filter == EVFILT_TIMER && event.ident == kTimer) batch.events[batch.count++] = {Kind::Timer, event.udata};
            else { batch.error = EPROTO; return batch; }
        }
        return batch;
    }
    int cancel(std::uint64_t) {
        kevent64_s timer{}; timer.ident = kTimer; timer.filter = EVFILT_TIMER; timer.flags = EV_DELETE;
        // A delivered one-shot has already deleted itself.
        return receipt(timer, ENOENT);
    }
} queue;
Protocol<Kqueue> protocol(queue);
}
Mode mode() { return selected.load(std::memory_order_relaxed); }
void configure() {
    Mode requested;
    if (!parseMode(std::getenv("PETARI_BATON_WAIT_MODE"), requested)) fatal("invalid-PETARI_BATON_WAIT_MODE", EINVAL);
    selected.store(requested, std::memory_order_release);
    if (requested == Mode::Legacy) return; // Original default: no timer/policy calls.
    mach_timebase_info_data_t info{};
    const auto result = mach_timebase_info(&info);
    if (result != KERN_SUCCESS || !info.numer || !info.denom) fatal("timebase", result ? result : EINVAL);
    timebase = {info.numer, info.denom};
    if (requested == Mode::Critical) {
        const int error = queue.initialize(); if (error) fatal("kqueue-initialize", error);
    }
    std::fprintf(stderr, "[baton wait] requested=%s effective=%s setup_error=0 timebase=%u/%u\n", name(requested), name(requested), info.numer, info.denom);
}
bool notificationAvailable() { return queue.published.load(std::memory_order_acquire) && queue.fd.load(std::memory_order_acquire) >= 0; }
void notify() {
    kevent64_s user{}; user.ident = kUser; user.filter = EVFILT_USER; user.fflags = NOTE_TRIGGER;
    const int result = kevent64(queue.fd.load(std::memory_order_acquire), &user, 1, nullptr, 0, 0, nullptr);
    if (result < 0) fatal("kqueue-notify", errno);
    if (result != 0) fatal("kqueue-notify-result", EPROTO);
}
void applyThreadPolicy() {
    if (mode() != Mode::Realtime) return;
    RealtimeContract contract;
    if (!realtimeContract(timebase, contract)) fatal("realtime-contract-conversion", EOVERFLOW);
    thread_time_constraint_policy_data_t requested{contract.period, contract.computation, contract.constraint, TRUE};
    const mach_port_t thread = mach_thread_self();
    const kern_return_t set = thread_policy_set(thread, THREAD_TIME_CONSTRAINT_POLICY,
        reinterpret_cast<thread_policy_t>(&requested), THREAD_TIME_CONSTRAINT_POLICY_COUNT);
    thread_time_constraint_policy_data_t actual{};
    mach_msg_type_number_t count = THREAD_TIME_CONSTRAINT_POLICY_COUNT; boolean_t defaults = FALSE;
    const kern_return_t get = thread_policy_get(thread, THREAD_TIME_CONSTRAINT_POLICY,
        reinterpret_cast<thread_policy_t>(&actual), &count, &defaults);
    const kern_return_t deallocate = mach_port_deallocate(mach_task_self(), thread);
    std::fprintf(stderr, "[baton wait] realtime requested_us=500/250/500 requested_ticks=%u/%u/%u actual_ticks=%u/%u/%u set_error=%d get_error=%d deallocate_error=%d default=%d count=%u\n",
        requested.period, requested.computation, requested.constraint, actual.period, actual.computation, actual.constraint, set, get, deallocate, defaults, count);
    PETARI_GPU_TRACE(auto c = PetariNative::GpuTrace::monitorContext;
        c.detail = static_cast<unsigned>(get); PetariNative::GpuTrace::emitScheduling("monitor_rt_policy_result", c, {}, set);
        PetariNative::GpuTrace::emit("monitor_rt_requested_period_ticks", c, requested.period);
        PetariNative::GpuTrace::emit("monitor_rt_requested_computation_ticks", c, requested.computation);
        PetariNative::GpuTrace::emit("monitor_rt_requested_constraint_ticks", c, requested.constraint);
        PetariNative::GpuTrace::emit("monitor_rt_actual_period_ticks", c, actual.period);
        PetariNative::GpuTrace::emit("monitor_rt_actual_computation_ticks", c, actual.computation);
        PetariNative::GpuTrace::emit("monitor_rt_actual_constraint_ticks", c, actual.constraint));
    if (deallocate != KERN_SUCCESS) fatal("realtime-thread-port-release", deallocate);
    if (set != KERN_SUCCESS) fatal("realtime-policy-set", set);
    if (get != KERN_SUCCESS) fatal("realtime-policy-get", get);
    if (count != THREAD_TIME_CONSTRAINT_POLICY_COUNT || !sameContract(contract,
        {actual.period, actual.computation, actual.constraint}, defaults)) fatal("realtime-policy-readback", EPROTO);
}
void traceThreadPolicy() {
    if (mode() != Mode::Realtime) return;
    thread_time_constraint_policy_data_t actual{};
    mach_msg_type_number_t count = THREAD_TIME_CONSTRAINT_POLICY_COUNT; boolean_t defaults = FALSE;
    const kern_return_t result = thread_policy_get(pthread_mach_thread_np(pthread_self()), THREAD_TIME_CONSTRAINT_POLICY,
        reinterpret_cast<thread_policy_t>(&actual), &count, &defaults);
    auto c = PetariNative::GpuTrace::monitorContext; c.detail = defaults;
    PetariNative::GpuTrace::emitScheduling("monitor_rt_wake_policy", c, {}, result);
    PetariNative::GpuTrace::emit("monitor_rt_wake_period_ticks", c, actual.period);
    PetariNative::GpuTrace::emit("monitor_rt_wake_computation_ticks", c, actual.computation);
    PetariNative::GpuTrace::emit("monitor_rt_wake_constraint_ticks", c, actual.constraint);
    RealtimeContract requested;
    if (result != KERN_SUCCESS) fatal("realtime-wake-policy-get", result);
    if (count != THREAD_TIME_CONSTRAINT_POLICY_COUNT || !realtimeContract(timebase, requested) ||
        !sameContract(requested, {actual.period, actual.computation, actual.constraint}, defaults)) fatal("realtime-wake-policy-readback", EPROTO);
}
Result waitCritical(std::uint64_t ns, bool forcedPending) {
    struct Plan { std::uint64_t absolute, roundedNs; } plan;
    const auto calculate = [ns](std::uint64_t sample) {
        Plan result;
        if (!deadline(sample, ns, timebase, result.absolute, result.roundedNs)) fatal("deadline-conversion", EOVERFLOW);
        return result;
    };
    // One cached flag, complete trace-bearing body outlined. No initialized
    // trace clock locals (and no trace TLS) exist on the disabled edge. Both
    // branches compute the actual normal timer plan before it is used.
    if (PetariNative::GpuTrace::enabled.load(std::memory_order_relaxed)) {
        PetariNative::GpuTrace::detail::invokeEnabled([&] {
            const auto steadyBefore = PetariNative::GpuTrace::nowNs();
            const auto sample = mach_absolute_time();
            const auto steadyAfter = PetariNative::GpuTrace::nowNs();
            plan = calculate(sample);
            auto c = PetariNative::GpuTrace::monitorContext; c.detail = forcedPending;
            PetariNative::GpuTrace::emitAt("monitor_mach_sample_before", c, steadyBefore, sample);
            PetariNative::GpuTrace::emitAt("monitor_mach_sample_after", c, steadyAfter, sample);
            PetariNative::GpuTrace::emit("monitor_mach_deadline", c, plan.absolute);
            PetariNative::GpuTrace::emit("monitor_deadline_lower_ns", c, steadyBefore + plan.roundedNs);
            PetariNative::GpuTrace::emit("monitor_deadline_upper_ns", c, steadyAfter + plan.roundedNs);
            // Mapped UPPER bound, explicitly bracketed above; not a guessed offset.
            PetariNative::GpuTrace::waitBegin("kevent64-timer-mapped-upper-bound", steadyAfter + plan.roundedNs, c);
        });
    } else {
        plan = calculate(mach_absolute_time());
    }
    const Result result = protocol.wait(plan.absolute, forcedPending);
    PETARI_GPU_TRACE(PetariNative::GpuTrace::waitEnd();
        auto c = PetariNative::GpuTrace::monitorContext; c.detail = result.generation;
        PetariNative::GpuTrace::emit("monitor_critical_returns", c, result.returns);
        PetariNative::GpuTrace::emit("monitor_critical_interrupts", c, result.interrupts);
        PetariNative::GpuTrace::emit("monitor_critical_stale", c, result.stale);
        PetariNative::GpuTrace::emit("monitor_critical_cause", c, (result.timer ? 1 : 0) | (result.notification ? 2 : 0)));
    if (result.error) fatal(result.errorSite, result.error, result.cleanupError);
    if (result.cleanupError) fatal("cleanup", result.cleanupError);
    return result;
}
void afterForkChild() {
    static_assert(std::atomic<int>::is_always_lock_free && std::atomic<bool>::is_always_lock_free,
                  "fork child must not take a C++ atomic lock");
    queue.published.store(false, std::memory_order_relaxed);
    const int inherited = queue.fd.exchange(-1, std::memory_order_relaxed);
    if (inherited >= 0) close(inherited); // Child descriptor only; no locks/allocations.
}
}
