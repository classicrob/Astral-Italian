#pragma once
#include <atomic>
#include <cstdint>

namespace PetariNative {
namespace Detail {
inline std::atomic<int> gThreadFlushToZero{-1};
}

// Select before creating workers; Darwin pthread startup resets FPCR.
class FloatingPointThreadPolicy {
public:
    explicit FloatingPointThreadPolicy(bool enabled)
        : mPrevious(Detail::gThreadFlushToZero.exchange(enabled ? 1 : 0)) {}
    ~FloatingPointThreadPolicy() { Detail::gThreadFlushToZero.store(mPrevious); }
    FloatingPointThreadPolicy(const FloatingPointThreadPolicy&) = delete;
    FloatingPointThreadPolicy& operator=(const FloatingPointThreadPolicy&) = delete;

    static void apply() {
#if defined(__aarch64__)
        const int selected = Detail::gThreadFlushToZero.load();
        if (selected < 0) return;
        std::uint64_t control;
        asm volatile("mrs %0, fpcr" : "=r"(control));
        constexpr std::uint64_t fz = 1ULL << 24;
        control = selected ? control | fz : control & ~fz;
        asm volatile("msr fpcr, %0\n\tisb" : : "r"(control) : "memory");
#endif
    }
private:
    int mPrevious;
};
}
