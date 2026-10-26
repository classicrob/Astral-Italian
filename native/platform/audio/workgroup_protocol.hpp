#pragma once
#include <cstdint>
#include <petari/platform/audio_workgroup.hpp>

namespace PetariNative::Platform::AudioWorkgroup::Protocol {
// This ledger is the DSP production cycle, NOT an AIInitDMA cycle: the latter
// registers a previous mixed buffer and can occur before this work completes.
struct Cycle {
    Origin source{};
    std::uint64_t serial = 0, deadline = 0;
    unsigned subframes = 0, completed = 0, rate = 0;
    std::uintptr_t left = 0, right = 0;
    bool active = false;
    bool begin(Origin o, std::uint64_t n, unsigned count, unsigned hz,
               std::uint64_t ticksPerSecond, std::uintptr_t l, std::uintptr_t r) {
        if (active || !o.caller || !o.epoch || !o.queuedMach || !count || count > 255 ||
            (hz != 32000 && hz != 48000) || !ticksPerSecond || !l || !r || l == r) return false;
        source = o; serial = n; subframes = count; rate = hz; completed = 0; left = l; right = r;
        // 80 samples/subframe; existing Pacer's maximum advance is 5/4.
        const auto budget = __uint128_t(count) * 80 * ticksPerSecond * 4 / (std::uint64_t(hz) * 5);
        if (!budget || budget > UINT64_MAX || o.queuedMach > UINT64_MAX - budget) return false;
        deadline = o.queuedMach + static_cast<std::uint64_t>(budget);
        active = true;
        return true;
    }
    bool completeSubframe() {
        if (!active || completed == subframes) return false;
        ++completed;
        return true;
    }
    bool ready() const { return active && completed == subframes; }
    bool finish() { if (!ready()) return false; active = false; return true; }
    bool cancel() { const bool was = active; active = false; return was; }
};

// Pure join/leave ownership state, also used by the native API adapter.
struct Membership {
    std::uint64_t owner = 0, epoch = 0;
    bool joined = false;
    bool adopt(std::uint64_t self, std::uint64_t generation, int policyError, int joinError) {
        if (joined || !self || !generation || policyError || joinError) return false;
        owner = self; epoch = generation; joined = true; return true;
    }
    bool leave(std::uint64_t self) {
        if (!joined || owner != self) return false;
        joined = false; epoch = 0; return true;
    }
};
}
