#pragma once
// Same ring operations and buffers. Trace-only clock/host/TLS work stays
// behind a cached flag and an outlined boundary, including return values.
#include <petari/gpu_trace.hpp>
#include <cstddef>
#include <cstdint>
namespace PetariNative::GpuTrace::Audio {
// Single production device/ring. State is trace-only, never used for audio flow.
inline std::atomic<std::uint64_t> lastEntryNs{0}, lastExitNs{0}, activeCallbacks{0};
struct CallbackState { std::uint64_t span = 0, entry = 0, requested = 0; unsigned depth = 0; };
inline thread_local CallbackState callbackState;
template<class Ring>
[[gnu::noinline]] inline void callbackBeginEnabled(Ring& ring, int additionalBytes, int realtimeStatus) {
    const auto entry = nowNs();
    const auto active = activeCallbacks.fetch_add(1, std::memory_order_relaxed);
    const auto previousEntry = lastEntryNs.exchange(entry, std::memory_order_relaxed);
    const auto previousExit = lastExitNs.load(std::memory_order_relaxed);
    auto& state = callbackState;
    const bool nested = state.depth++ != 0;
    if (!nested) { state.span = nextId(); state.entry = entry;
        state.requested = additionalBytes > 0 ? (static_cast<std::uint64_t>(additionalBytes) + 3) / 4 : 0; }
    Context c{}; c.span = state.span; c.subject = reinterpret_cast<std::uintptr_t>(&ring);
    Attributes a; auto& x = a.audio; x.kind = AudioObservation::Kind::CallbackEnter;
    x.beginNs = x.endNs = entry; x.callback = state.span;
    x.previousEntryNs = previousEntry; x.previousExitNs = previousExit;
    x.activeBefore = active; x.protocolError = nested || active != 0;
    x.requested = state.requested; x.realtimeStatus = realtimeStatus;
    x.level = ring.level(); x.levelSampleNs = nowNs();
    emitSchedulingAt(entry, "audio_device_callback_entry", c, a);
}
template<class Ring>
[[gnu::noinline]] inline void callbackEndEnabled(Ring& ring, int realtimeStatus) {
    const auto exit = nowNs();
    auto& state = callbackState;
    const auto active = activeCallbacks.fetch_sub(1, std::memory_order_relaxed);
    const bool invalid = state.depth != 1 || active != 1;
    Context c{}; c.span = state.span; c.subject = reinterpret_cast<std::uintptr_t>(&ring);
    Attributes a; auto& x = a.audio; x.kind = AudioObservation::Kind::CallbackExit;
    x.beginNs = state.entry; x.endNs = exit; x.callback = state.span;
    x.activeBefore = active; x.protocolError = invalid; x.requested = state.requested;
    x.realtimeStatus = realtimeStatus; x.level = ring.level(); x.levelSampleNs = nowNs();
    if (state.depth && --state.depth == 0) { lastExitNs.store(exit, std::memory_order_relaxed); state.span = 0; }
    emitSchedulingAt(exit, "audio_device_callback_exit", c, a);
}
template<class Ring>
inline std::size_t read(Ring& ring, std::int16_t* output, std::size_t count) {
    std::size_t copied = 0;
    if (enabled.load(std::memory_order_relaxed)) {
        detail::invokeEnabled([&] {
            const auto begin = nowNs();
            copied = ring.read(output, count);
            const auto end = nowNs();
            if (copied < count) {
                Context c{}; c.span = nextId(); c.subject = reinterpret_cast<std::uintptr_t>(&ring);
                Attributes a; a.audio.kind = AudioObservation::Kind::Read;
                a.audio.beginNs = begin; a.audio.endNs = end; a.audio.callback = callbackState.span;
                a.audio.requested = count; a.audio.copied = copied; a.audio.missing = count - copied;
                emitSchedulingAt(end, "audio_device_ring_shortfall", c, a, count - copied);
            }
        });
    } else {
        copied = ring.read(output, count);
    }
    return copied; // exact native read result; no counter sampling or extra read
}
template<class Ring>
[[gnu::noinline]] inline std::uint64_t reportEnabled(Ring& ring, const char* source) {
    const auto begin = nowNs();
    const auto missing = ring.takeUnderrunFrames();
    const auto end = nowNs();
    Context c{}; c.span = nextId(); c.subject = reinterpret_cast<std::uintptr_t>(&ring);
    Attributes a; a.audio.kind = AudioObservation::Kind::Report;
    a.audio.beginNs = begin; a.audio.endNs = end; a.audio.missing = missing; a.audio.reportSource = source;
    emitSchedulingAt(end, "audio_underrun_report_exchange", c, a, missing);
    return missing; // The existing [audio] line prints this EXACT count.
}
template<class Ring>
inline std::uint64_t takeUnderruns(Ring& ring, const char* source = "periodic") {
    if (enabled.load(std::memory_order_relaxed)) return reportEnabled(ring, source);
    return ring.takeUnderrunFrames();
}
}
