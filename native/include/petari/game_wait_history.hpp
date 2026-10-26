#pragma once
// Independent default-off diagnostic bridge. Never changes a wait, predicate,
// timeout, priority, lock order or callback. No dependency on audio history.
#include <atomic>
#include <cstdint>
#include <utility>

namespace PetariNative::GameWaitHistory {
enum class Kind : std::uint64_t {
    Phase=1, NearMiss, Trigger, InvalidPhase, Csv, InterruptMutex,
    InterruptOwner, CpuWait, BatonGrant, CpuResumed, Sleep, Ready,
    HostWork, Token, Fifo, DrawDone, EventPump, HandleEvents, Compose,
    EndFrame, BeginFrame, DisplayUpdate, Drawable, Present, Submit,
    FrameSlot, Staging, RenderQueue, RenderSync, Snapshot, Capture,
    Worker, RunState, Vm, Writer, QueueDone, MapCallback, EncoderFinish, WindowState, ThreadBind, Count
};
inline constexpr const char* kindNames[] = {
    "unknown", "phase", "near-miss", "trigger", "invalid-phase", "csv", "interrupt-mutex",
    "interrupt-owner", "cpu-wait", "baton-grant", "cpu-resumed", "sleep", "ready",
    "host-work", "token", "fifo", "draw-done", "event-pump", "handle-events", "compose",
    "end-frame", "begin-frame", "display-update", "drawable", "present", "submit",
    "frame-slot", "staging", "render-queue", "render-sync", "snapshot", "capture",
    "worker", "run-state", "vm", "writer", "queue-done", "map-callback", "encoder-finish",
    "window-state", "thread-bind"
};
static_assert(sizeof(kindNames)/sizeof(kindNames[0])==static_cast<unsigned>(Kind::Count));
inline const char* kindName(Kind kind) noexcept {
    const auto index=static_cast<unsigned>(kind);
    return index<static_cast<unsigned>(Kind::Count)?kindNames[index]:"unknown";
}
struct Token {
    std::uint64_t start=0, kind=0, object=0, value=0, id=0, depth=0;
};
struct Hooks {
    Token (*begin)(Kind, std::uint64_t, std::uint64_t) noexcept;
    void (*end)(const Token&) noexcept;
    void (*event)(Kind, std::uint64_t, std::uint64_t, std::uint64_t) noexcept;
    void (*mark)(int, std::uint64_t, std::uint64_t) noexcept;
    void (*finish)() noexcept;
};
inline std::atomic<bool> enabled{false};
inline std::atomic<const Hooks*> hooks{nullptr};
// Function-local scope exists ONLY on the enabled branch. Disabled code calls
// the original operation directly; no token, clock, allocation or destructor.
template<class Work> decltype(auto) invoke(Kind kind, std::uint64_t object,
                                           std::uint64_t value, Work&& work) {
    if (!enabled.load(std::memory_order_relaxed)) return std::forward<Work>(work)();
    const auto* h=hooks.load(std::memory_order_acquire);
    if (!h) return std::forward<Work>(work)();
    struct Exit { const Hooks* h; Token token; ~Exit() { h->end(token); } } exit{h,h->begin(kind,object,value)};
    return std::forward<Work>(work)();
}
inline void event(Kind kind, std::uint64_t object=0, std::uint64_t value=0, std::uint64_t detail=0) noexcept {
    if (!enabled.load(std::memory_order_relaxed)) return;
    if (const auto* h=hooks.load(std::memory_order_acquire)) h->event(kind,object,value,detail);
}
inline void mark(int mark, std::uint64_t frame, std::uint64_t stamp=0) noexcept {
    if (!enabled.load(std::memory_order_relaxed)) return;
    if (const auto* h=hooks.load(std::memory_order_acquire)) h->mark(mark,frame,stamp);
}
inline void finish() noexcept {
    if (!enabled.load(std::memory_order_relaxed)) return;
    if (const auto* h=hooks.load(std::memory_order_acquire)) h->finish();
}
} // namespace PetariNative::GameWaitHistory

// Gate argument evaluation too: no bound-thread lookup or metadata work when OFF.
#define PETARI_GAME_WAIT_EVENT(...) do { \
    if (::PetariNative::GameWaitHistory::enabled.load(std::memory_order_relaxed)) \
        ::PetariNative::GameWaitHistory::event(__VA_ARGS__); \
} while (false)
