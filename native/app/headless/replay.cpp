#include "replay.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
#include <string>

namespace PetariNative::Headless {
namespace {
std::uint64_t number(std::istream& in, unsigned bytes) {
    std::uint64_t result = 0;
    for (unsigned i = 0; i < bytes; ++i) {
        const int ch = in.get();
        if (ch == std::char_traits<char>::eof()) throw ReplayError("truncated KPAD tape");
        result = (result << 8) | static_cast<unsigned char>(ch);
    }
    return result;
}
std::int32_t signed32(std::istream& in) {
    const std::uint32_t bits = static_cast<std::uint32_t>(number(in, 4));
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}
}
Replay::Replay(std::istream& in, bool inputsFromSync) : mWaitingForSync(inputsFromSync) {
    for (unsigned channel = 0; channel < 4; ++channel) {
        auto& read = mBootstrap[channel];
        read = {};
        read.channel = channel;
        const bool connected = channel == 0;
        read.returned = connected ? 1 : 0;
        read.error = connected ? 0 : -2;
        read.probeError = connected ? 0 : -1;
        read.device = connected ? 1 : 253;
        read.samples.resize(1);
        auto& sample = read.samples[0];
        auto bits = [&](unsigned offset, std::uint32_t value) {
            for (unsigned i = 0; i < 4; ++i) sample[offset + i] = value >> (24 - 8 * i);
        };
        bits(16, 0xbf800000);
        for (unsigned offset : {24, 52, 84}) bits(offset, 0x3f800000);
        sample[92] = connected ? 1 : 253;
        sample[93] = connected ? 0 : 255;
        sample[95] = connected ? 5 : 0;
        if (connected) {
            bits(108, 0xbf800000);
            bits(116, 0x3f800000);
        }
    }
    char magic[8];
    if (!in.read(magic, sizeof(magic)) || std::memcmp(magic, "SMG2KPD1", 8)) throw ReplayError("bad KPAD tape magic");
    mFrames = number(in, 8);
    const auto count = number(in, 8);
    if (!mFrames || mFrames > 10000000 || count > 40000000) throw ReplayError("KPAD tape bounds exceeded");
    for (std::uint64_t i = 0; i < count; ++i) {
        Read read{};
        read.frame = number(in, 8);
        read.ordinal = static_cast<std::uint32_t>(number(in, 4));
        read.channel = static_cast<std::uint32_t>(number(in, 4));
        read.capacity = static_cast<std::uint32_t>(number(in, 4));
        read.returned = signed32(in);
        read.error = signed32(in);
        read.probeError = signed32(in);
        read.device = static_cast<std::uint32_t>(number(in, 4));
        const auto samples = number(in, 4);
        if (read.frame >= mFrames || read.channel >= 4 || !read.capacity || read.capacity > 4096 ||
            read.returned > static_cast<std::int32_t>(read.capacity) || read.returned < -128 ||
            samples != static_cast<std::uint64_t>(std::max(read.returned, 1)) || read.device > 255 ||
            read.probeError < -128 || read.probeError > 127) throw ReplayError("invalid KPAD read bounds");
        if ((!mReads.empty() && (read.frame < mReads.back().frame ||
            (read.frame == mReads.back().frame && read.ordinal != mReads.back().ordinal + 1))) ||
            ((mReads.empty() || read.frame != mReads.back().frame) && read.ordinal != 0))
            throw ReplayError("KPAD reads must be ordered with contiguous per-frame ordinals");
        read.samples.resize(static_cast<std::size_t>(samples));
        for (auto& sample : read.samples) {
            if (!in.read(reinterpret_cast<char*>(sample.data()), sample.size())) throw ReplayError("truncated KPAD sample");
        }
        for (auto j = mReads.size(); j > 0 && mReads[j - 1].frame == read.frame; --j) {
            const auto& old = mReads[j - 1];
            if (old.channel == read.channel && (old.probeError != read.probeError || old.device != read.device))
                throw ReplayError("within-frame connection transitions need timestamped raw replay");
        }
        mReads.push_back(std::move(read));
    }
    if (in.peek() != std::char_traits<char>::eof()) throw ReplayError("trailing KPAD tape data");
}
const Read& Replay::consume(std::uint32_t channel, std::uint32_t capacity) {
    if (mWaitingForSync) {
        if (channel >= 4 || !capacity || capacity > 4096) throw ReplayError("invalid pre-sync KPAD read");
        auto& read = mBootstrap[channel];
        read.capacity = capacity;
        return read;
    }
    if (finished() || mNext == mReads.size()) throw ReplayError("extra KPAD read at frame " + std::to_string(mFrame));
    const auto& read = mReads[mNext];
    if (read.frame != mFrame || read.ordinal != mOrdinal || read.channel != channel || read.capacity != capacity)
        throw ReplayError("KPAD frame/read/channel/capacity mismatch at frame " + std::to_string(mFrame) +
                          " read " + std::to_string(mOrdinal));
    ++mNext;
    ++mOrdinal;
    return read;
}
void Replay::setSegments(std::vector<std::uint64_t> lengths) {
    std::uint64_t end = 0;
    mSegmentEnds.clear();
    for (const auto length : lengths) {
        if (!length) throw ReplayError("empty tape segment");
        end += length;
        mSegmentEnds.push_back(end);
    }
    if (end != mFrames) throw ReplayError("tape segments must cover exactly the tape frames");
}
void Replay::endFrame() {
    mSegmentJustEnded = false;
    if (mWaitingForSync) return;
    if (finished()) throw ReplayError("extra game frame after end of KPAD tape");
    if (mNext < mReads.size() && mReads[mNext].frame == mFrame) throw ReplayError("missing KPAD reads at frame " + std::to_string(mFrame));
    ++mFrame;
    mFrameStart = mNext;
    mOrdinal = 0;
    if (!mSegmentEnds.empty() && mFrame == mSegmentEnds[mSegment] && mFrame != mFrames) {
        ++mSegment;
        mWaitingForSync = true;  // the next segment starts at its own sync seam
        mSegmentJustEnded = true;
    }
}
const Read* Replay::connection(std::uint32_t channel) const {
    if (mWaitingForSync) return channel < 4 ? &mBootstrap[channel] : nullptr;
    for (auto i = mFrameStart; i < mReads.size() && mReads[i].frame == mFrame; ++i)
        if (mReads[i].channel == channel) return &mReads[i];
    return nullptr;
}
FrameClock::FrameClock(std::uint32_t n, std::uint32_t d) : mNumerator(n), mDenominator(d) {
    if (!n || !d || n > 1000000 || d > 1000000 || n < d || static_cast<std::uint64_t>(n) > 1000ULL*d)
        throw ReplayError("fixed frame rate must be in 1..1000 Hz");
}
std::uint64_t FrameClock::advance() {
    const auto numerator = static_cast<std::uint64_t>(60750000) * mDenominator + mRemainder;
    const auto delta = numerator / mNumerator;
    if (mTicks > std::numeric_limits<std::uint64_t>::max() - delta) throw ReplayError("virtual clock overflow");
    mRemainder = numerator % mNumerator;
    mTicks += delta;
    return mTicks;
}
}
