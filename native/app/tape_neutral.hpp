#pragma once
// A tape-owned policy, never a replay environment override. No sample is edited.
#include "headless/replay.hpp"
#include <array>
#include <string>

namespace PetariNative::App::TapeNeutral {
enum class Policy { SingleBootstrap, NativeControls };
inline const char* name(Policy policy) {
    return policy == Policy::NativeControls ? "native-neutral-controls-v1" : "single-bootstrap-v1";
}
inline Policy parse(const std::string& value) {
    if (value == name(Policy::SingleBootstrap)) return Policy::SingleBootstrap;
    if (value == name(Policy::NativeControls)) return Policy::NativeControls;
    throw Headless::ReplayError("unknown neutral-tail policy");
}
inline std::uint32_t word(const Headless::Sample& sample, unsigned at) {
    return (std::uint32_t(sample[at]) << 24) | (std::uint32_t(sample[at + 1]) << 16) |
           (std::uint32_t(sample[at + 2]) << 8) | sample[at + 3];
}
inline bool magnitude(const Headless::Sample& sample, unsigned at) { return (word(sample, at) & 0x7fffffff) != 0; }
inline bool hasControl(const Headless::Sample& sample) {
    if (word(sample, 0) || word(sample, 4) || word(sample, 8) || (sample[94] > 0 && sample[94] < 128)) return true;
    if (sample[92] == 1 || sample[92] == 6) return magnitude(sample, 96) || magnitude(sample, 100);
    if (sample[92] == 2 || sample[92] == 7) {
        if (word(sample, 96) || word(sample, 100) || word(sample, 104)) return true;
        for (unsigned at = 108; at <= 128; at += 4) if (magnitude(sample, at)) return true;
    }
    return false;
}
class NativeTail {
public:
    void validate(const Headless::Read& read) {
        if (read.channel >= mChannels.size() || read.samples.empty()) fail("missing native tail channel/sample");
        auto& old = mChannels[read.channel];
        if (old.seen && (read.capacity != old.capacity || read.error != old.error ||
            read.probeError != old.probeError || read.device != old.device)) fail("native tail connection/control envelope changed");
        for (const auto& sample : read.samples) {
            if (hasControl(sample)) fail("native tail has button/stick/pointer control");
            if (magnitude(sample, 28) || magnitude(sample, 176) || magnitude(sample, 180) || magnitude(sample, 184) ||
                ((sample[92] == 1 || sample[92] == 6) && magnitude(sample, 120))) fail("native tail has accel/gyro event");
            if (old.seen && sample != old.sample) fail("native tail sample changed from first recorded rest sample");
            if (!old.seen) {
                old.seen = true; old.sample = sample; old.capacity = read.capacity;
                old.error = read.error; old.probeError = read.probeError; old.device = read.device;
            }
        }
        // Only frame, ordinal, returned and sample count may advance. Every
        // sample's complete 240 bytes remain exact, including unknown fields.
    }
private:
    [[noreturn]] static void fail(const char* why) { throw Headless::ReplayError(why); }
    struct Channel {
        bool seen = false;
        std::uint32_t capacity = 0, device = 0;
        std::int32_t error = 0, probeError = 0;
        Headless::Sample sample{};
    };
    std::array<Channel, 4> mChannels{};
};
}
