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
Replay::Replay(std::istream& in) {
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
    if (finished() || mNext == mReads.size()) throw ReplayError("extra KPAD read at frame " + std::to_string(mFrame));
    const auto& read = mReads[mNext];
    if (read.frame != mFrame || read.ordinal != mOrdinal || read.channel != channel || read.capacity != capacity)
        throw ReplayError("KPAD frame/read/channel/capacity mismatch at frame " + std::to_string(mFrame) +
                          " read " + std::to_string(mOrdinal));
    ++mNext;
    ++mOrdinal;
    return read;
}
void Replay::endFrame() {
    if (finished()) throw ReplayError("extra game frame after end of KPAD tape");
    if (mNext < mReads.size() && mReads[mNext].frame == mFrame) throw ReplayError("missing KPAD reads at frame " + std::to_string(mFrame));
    ++mFrame;
    mFrameStart = mNext;
    mOrdinal = 0;
}
const Read* Replay::connection(std::uint32_t channel) const {
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
