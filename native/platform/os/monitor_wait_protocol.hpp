#pragma once
// Platform-free state machine. Only its Backend performs blocking/system calls.
#include <array>
#include <atomic>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <limits>

namespace PetariNative::Platform::MonitorWait {
enum class Mode : unsigned { Legacy, Critical, Realtime };
inline bool parseMode(const char* text, Mode& mode) {
    if (!text || !std::strcmp(text, "legacy")) mode = Mode::Legacy;
    else if (!std::strcmp(text, "critical")) mode = Mode::Critical;
    else if (!std::strcmp(text, "realtime")) mode = Mode::Realtime;
    else return false; // No combined mode is authorized.
    return true;
}
inline const char* name(Mode mode) {
    switch (mode) {
    case Mode::Legacy: return "legacy";
    case Mode::Critical: return "critical";
    case Mode::Realtime: return "realtime";
    }
    return "invalid";
}
struct Timebase { std::uint32_t numer = 0, denom = 0; };
inline bool nsToTicks(std::uint64_t ns, Timebase scale, std::uint64_t& ticks) {
    if (!scale.numer || !scale.denom) return false;
    const auto wide = static_cast<unsigned __int128>(ns) * scale.denom;
    const auto rounded = (wide + scale.numer - 1) / scale.numer;
    if (rounded > std::numeric_limits<std::uint64_t>::max()) return false;
    ticks = static_cast<std::uint64_t>(rounded); return true;
}
inline bool ticksToNs(std::uint64_t ticks, Timebase scale, std::uint64_t& ns) {
    if (!scale.numer || !scale.denom) return false;
    const auto wide = static_cast<unsigned __int128>(ticks) * scale.numer;
    const auto rounded = (wide + scale.denom - 1) / scale.denom;
    if (rounded > std::numeric_limits<std::uint64_t>::max()) return false;
    ns = static_cast<std::uint64_t>(rounded); return true;
}
inline bool deadline(std::uint64_t now, std::uint64_t ns, Timebase scale,
                     std::uint64_t& absolute, std::uint64_t& roundedNs) {
    std::uint64_t ticks = 0;
    if (!nsToTicks(ns, scale, ticks) || !ticksToNs(ticks, scale, roundedNs) ||
        now > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) ||
        ticks > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) - now) return false;
    absolute = now + ticks; return true;
}
struct RealtimeContract { std::uint32_t period = 0, computation = 0, constraint = 0; };
inline bool realtimeContract(Timebase scale, RealtimeContract& contract) {
    std::uint64_t period = 0, computation = 0, constraint = 0;
    if (!nsToTicks(500000, scale, period) || !nsToTicks(250000, scale, computation) ||
        !nsToTicks(500000, scale, constraint) || !period || !computation ||
        period > UINT32_MAX || computation > UINT32_MAX || constraint > UINT32_MAX) return false;
    contract = {static_cast<std::uint32_t>(period), static_cast<std::uint32_t>(computation), static_cast<std::uint32_t>(constraint)};
    return true;
}
inline bool sameContract(RealtimeContract requested, RealtimeContract actual, bool defaults) {
    return !defaults && requested.period == actual.period && requested.computation == actual.computation &&
           requested.constraint == actual.constraint;
}
// Same coalescing contract as the original semaphore flag. Clear on delivery
// BEFORE inspecting pending/holder state; notifications coalesced before clear
// are represented by that current state, and transitions after clear queue anew.
class NotificationGate {
public:
    template<class Signal> void request(bool available, Signal signal) {
        if (available && !pending_.exchange(true)) signal();
    }
    void consumed() { pending_.store(false); }
private:
    std::atomic<bool> pending_{false};
};
enum class Kind { Timer, Notification };
struct Event { Kind kind = Kind::Timer; std::uint64_t generation = 0; };
struct Batch {
    std::array<Event, 2> events{};
    unsigned count = 0;
    int error = 0;
    bool interrupted = false;
};
struct Result {
    std::uint64_t generation = 0, deadline = 0;
    unsigned returns = 0, interrupts = 0, stale = 0;
    bool timer = false, notification = false;
    int error = 0, cleanupError = 0;
    const char* errorSite = nullptr;
};
// One timer at a time; all timer mutations belong to the monitor. Notification
// may arrive concurrently, including before arm/receive or during cancellation.
// Backend receive BLOCKS; EINTR/stale retries never move the original deadline.
template<class Backend> class Protocol {
public:
    explicit Protocol(Backend& backend, std::uint64_t initialGeneration = 0) : backend_(backend), generation_(initialGeneration) {}
    Result wait(std::uint64_t absolute, bool critical) {
        Result result; result.deadline = absolute;
        if (generation_ == UINT64_MAX) { result.error = EOVERFLOW; result.errorSite = "generation"; return result; }
        result.generation = ++generation_;
        const int armed = backend_.arm(absolute, result.generation, critical, 0);
        if (armed) {
            result.error = armed; result.errorSite = "arm";
            result.cleanupError = backend_.cancel(result.generation); return result;
        }
        for (;;) {
            const Batch batch = backend_.receive(); ++result.returns;
            if (batch.interrupted) { ++result.interrupts; continue; }
            if (batch.error) return fail(result, batch.error, "receive");
            if (!batch.count || batch.count > batch.events.size()) return fail(result, EPROTO, "batch");
            for (unsigned i = 0; i < batch.count; ++i) {
                const auto& event = batch.events[i];
                if (event.kind == Kind::Notification) result.notification = true;
                else if (event.generation == result.generation) result.timer = true;
                else ++result.stale;
            }
            if (result.timer || result.notification) {
                // Even after a timer event, cancel acknowledges ENOENT and also
                // protects the next generation if the backend did not retire it.
                const int cancelled = backend_.cancel(result.generation);
                if (cancelled) { result.error = cancelled; result.errorSite = "cancel"; }
                return result;
            }
            // A finite protocol-error bound, not a timing/poll/backoff constant.
            if (result.stale > 8) return fail(result, EPROTO, "stale-generation");
        }
    }
private:
    Result fail(Result result, int error, const char* site) {
        result.error = error; result.errorSite = site;
        result.cleanupError = backend_.cancel(result.generation); return result;
    }
    Backend& backend_;
    std::uint64_t generation_ = 0;
};
} // namespace PetariNative::Platform::MonitorWait
