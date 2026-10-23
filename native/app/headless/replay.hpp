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
    explicit Replay(std::istream& input, bool inputsFromSync = false);
    bool waitingForSync() const { return mWaitingForSync; }
    void startInputs() { mWaitingForSync = false; }
    // Several tape segments (one per scene visit, each starting at its own sync seam): after the last frame of a segment the
    // replay waits for the next sync again (neutral boot-style reads); frames stay globally contiguous.
    void setSegments(std::vector<std::uint64_t> lengths);
    // Keep only the reads of the given segments of a whole-session tape (a segment is absolute game loops start+1..end), rebased
    // back to back, and make them the segments: the in-memory form of tools/port/replay_tape.py's trimmed tape, so the GX app
    // replays the recorded tape directly.
    struct SegmentRange { std::uint64_t start, end; };
    void keepSegments(const std::vector<SegmentRange>& ranges);
    bool segmentEnded() const { return mSegmentJustEnded; }
    const Read& consume(std::uint32_t channel, std::uint32_t capacity);
    void endFrame();
    std::uint64_t frame() const { return mFrame; }
    std::uint64_t frames() const { return mFrames; }
    bool finished() const { return mFrame == mFrames; }
    const Read* connection(std::uint32_t channel) const;
    // A neutral read (controller 0 connected, no buttons) for the loops between segments and after a replay was given up.
    const Read& neutral(std::uint32_t channel, std::uint32_t capacity);
private:
    std::vector<Read> mReads;
    bool mWaitingForSync = false;
    std::array<Read, 4> mBootstrap;
    std::uint64_t mFrames = 0, mFrame = 0;
    std::size_t mNext = 0, mFrameStart = 0;
    std::uint32_t mOrdinal = 0;
    std::vector<std::uint64_t> mSegmentEnds;  // global frame at which each segment ends (exclusive)
    std::size_t mSegment = 0;
    bool mSegmentJustEnded = false;
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
