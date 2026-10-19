#pragma once
#include <array>
#include <cstdint>
#include <istream>
#include <stdexcept>
#include <vector>

namespace PetariNative::Headless {
class ReplayError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
using Sample = std::array<std::uint8_t, 240>;
struct Read {
    std::uint64_t frame;
    std::uint32_t ordinal, channel, capacity;
    std::int32_t returned, error, probeError;
    std::uint32_t device;
    std::vector<Sample> samples;
};
class Replay {
public:
    explicit Replay(std::istream& input);
    const Read& consume(std::uint32_t channel, std::uint32_t capacity);
    void endFrame();
    std::uint64_t frame() const { return mFrame; }
    std::uint64_t frames() const { return mFrames; }
    bool finished() const { return mFrame == mFrames; }
    const Read* connection(std::uint32_t channel) const;
private:
    std::vector<Read> mReads;
    std::uint64_t mFrames = 0, mFrame = 0;
    std::size_t mNext = 0, mFrameStart = 0;
    std::uint32_t mOrdinal = 0;
};
class FrameClock {
public:
    explicit FrameClock(std::uint32_t rateNumerator = 60000, std::uint32_t rateDenominator = 1001);
    std::uint64_t advance();
    std::uint64_t ticks() const { return mTicks; }
private:
    std::uint32_t mNumerator, mDenominator;
    std::uint64_t mTicks = 0, mRemainder = 0;
};
}
