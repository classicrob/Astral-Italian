#pragma once
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>

namespace PetariNative::AudioOutputAU::Detail {
static_assert(std::atomic<std::uint64_t>::is_always_lock_free, "render atomics must not lock");
static_assert(std::atomic<void*>::is_always_lock_free, "render pointers must not lock");

// One producer / one consumer. Configuration/reset/destruction require control
// quiescence. All storage, including AU null-buffer scratch, is prefaulted.
class Ring {
public:
    bool configure(std::size_t capacity, std::size_t base, unsigned channels) {
        if (!capacity || capacity > 262144 || base >= capacity || (channels != 1 && channels != 2)) return false;
        samples.reset(new float[capacity * channels]{});
        size = capacity; planes = channels; baseFrames = base;
        readIndex.store(0); writeIndex.store(base); largest.store(0); minimum.store(UINT64_MAX);
        return true;
    }
    std::uint64_t level() const {
        const auto r=readIndex.load(std::memory_order_acquire);
        return writeIndex.load(std::memory_order_acquire)-r;
    }
    std::uint64_t target() const { return std::min<std::uint64_t>(baseFrames + largest.load(std::memory_order_relaxed), size); }
    // Producer owns writeIndex; sample it before the consumer's readIndex.
    std::uint64_t space() const {
        const auto w=writeIndex.load(std::memory_order_relaxed);
        const auto n=w-readIndex.load(std::memory_order_acquire);
        return n<=size ? size-n : 0;
    }
    std::uint64_t lowWater(double deviceRate, unsigned guestRate) const {
        return largest.load(std::memory_order_relaxed) + static_cast<std::uint64_t>(256.0 * deviceRate / guestRate);
    }
    bool write(const float* left, const float* right, std::size_t count) {
        if (count > space() || !left || (planes == 2 && !right)) return false;
        const auto w = writeIndex.load(std::memory_order_relaxed);
        for (std::size_t n = 0; n < count; ++n) {
            samples[(w+n)%size] = left[n];
            if (planes == 2) samples[size+(w+n)%size] = right[n];
        }
        writeIndex.store(w+count, std::memory_order_release); return true;
    }
    // Caller validates destination buffers and frame limit. No allocation,
    // lock, clock, framework or TLS. Single-consumer extrema need no CAS loop.
    std::size_t read(float* left, float* right, std::size_t count, bool mute) noexcept {
        if (count > largest.load(std::memory_order_relaxed)) largest.store(count, std::memory_order_relaxed);
        const auto r = readIndex.load(std::memory_order_relaxed);
        const auto w = writeIndex.load(std::memory_order_acquire);
        const auto available = w-r;
        const auto got = available <= size ? std::min<std::uint64_t>(count, available) : 0;
        for (std::size_t n = 0; n < count; ++n) {
            left[n] = !mute && n < got ? samples[(r+n)%size] : 0.f;
            if (planes == 2) right[n] = !mute && n < got ? samples[size+(r+n)%size] : 0.f;
        }
        readIndex.store(r+got, std::memory_order_release);
        const auto remaining = available <= size ? available-got : 0;
        if (remaining < minimum.load(std::memory_order_relaxed)) minimum.store(remaining, std::memory_order_relaxed);
        return got;
    }
    std::size_t capacity() const { return size; }
    unsigned channels() const { return planes; }
    std::uint64_t consumed() const { return readIndex.load(std::memory_order_acquire); }
    std::uint64_t largestRequest() const { return largest.load(std::memory_order_relaxed); }
    std::uint64_t minimumLevel() const {
        const auto value=minimum.load(std::memory_order_relaxed);
        return value==UINT64_MAX ? baseFrames : value;
    }
    std::uint64_t initialFrames() const { return baseFrames; }
private:
    std::unique_ptr<float[]> samples;
    std::size_t size = 0, baseFrames = 0;
    unsigned planes = 0;
    alignas(64) std::atomic<std::uint64_t> readIndex{0};
    alignas(64) std::atomic<std::uint64_t> writeIndex{0};
    std::atomic<std::uint64_t> largest{0}, minimum{0};
};

// Counts ALL guest audio ahead of the consumer, including converter-held and
// input/output staging, rather than treating hidden staging as free credit.
struct ProductionLedger {
    unsigned guestRate = 0;
    double deviceRate = 0;
    std::uint64_t pulled = 0, accepted = 0, produced = 0;
    double ahead(const Ring& ring) const {
        return std::max(0.0, static_cast<double>(pulled) +
            (static_cast<double>(ring.initialFrames())-ring.consumed()) * guestRate / deviceRate);
    }
    std::size_t wanted(const Ring& ring, std::size_t allowance, std::size_t quantum) const {
        if (!guestRate || !std::isfinite(deviceRate) || deviceRate <= 0) return 0;
        const double missing = ring.target() * guestRate / deviceRate - ahead(ring);
        if (missing < 1.0) return 0;
        return std::min<std::size_t>({allowance, quantum, static_cast<std::size_t>(missing)});
    }
};

// Off-thread report conversion. Cumulative rounding, never one lossy rounding
// per callback/report. Any nonzero raw shortfall remains a nonzero failure.
class ShortfallUnits {
public:
    std::uint64_t take(std::uint64_t rawTotal, unsigned guestRate, double deviceRate) {
        const long double exact = static_cast<long double>(rawTotal) * guestRate / deviceRate;
        const auto total = static_cast<std::uint64_t>(std::ceil(exact));
        const auto delta = total >= reported ? total-reported : 0;
        reported = total; fractional = exact-std::floor(exact); return delta;
    }
    long double remainder() const { return fractional; }
private:
    std::uint64_t reported = 0;
    long double fractional = 0;
};

enum Fault : std::uint64_t {
    BadBuffers=1, OverLimit=2, Reentrant=4, BadTimeline=8, MissingTimestamp=16,
    RingCorrupt=32, NoCallbacks=64, ConverterError=128, DeviceError=256, BadHostTimeline=512
};
struct RenderEvent {
    std::uint64_t mach=0, ticket=0, generation=0, value=0;
    unsigned kind=0;
};
// No query/TLS in this ABI. The configured publisher is itself a tested,
// bounded in-memory write, never an arbitrary user callback.
using Publish = void (*)(const RenderEvent&) noexcept;
using ReadClock = std::uint64_t (*)() noexcept;
struct RenderData {
    Ring ring;
    std::unique_ptr<float[]> scratch;
    std::size_t maxFrames=0;
    std::uint64_t generation=0, lastExit=0, callbackTicket=0;
    std::uint64_t gapTriggerTicks=0;
    bool mute=false, observing=false;
    ReadClock clock=nullptr;
    Publish publish=nullptr;
    bool timelineStarted=false;
    double nextSample=0;
    bool hostStarted=false;
    double hostTicksPerFrame=0,lastRateScalar=1;
    std::uint64_t previousHost=0,previousFrames=0;
    std::atomic<std::uint64_t> callbacks{0}, requested{0}, copied{0}, shortfalls{0};
    std::atomic<std::uint64_t> timelineFaults{0}, invalidTimestamps{0}, faults{0};
    std::atomic<std::uint64_t> lastFrames{0},hostTimelineFaults{0},badHostDelta{0},expectedHostDelta{0};
    std::atomic<std::uint64_t> sampleBits{0}, hostTime{0}, maxGapTicks{0};
};
// Owner outlives the AudioUnit AND every in-flight callback. Seq-cst entry /
// unpublish / reader-count order prevents reclaiming a reader's generation.
struct RenderOwner {
    std::atomic<RenderData*> published{nullptr};
    std::atomic<std::uint64_t> readers{0}, faults{0};
    std::atomic_flag consumer=ATOMIC_FLAG_INIT;
};
}
