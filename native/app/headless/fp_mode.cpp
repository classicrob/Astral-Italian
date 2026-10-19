#include "fp_mode.hpp"
#include <stdexcept>
namespace PetariNative::Headless {
namespace {
std::uint64_t readControl() {
#if defined(__aarch64__)
    std::uint64_t value;
    asm volatile("mrs %0, fpcr" : "=r"(value));
    return value;
#else
    throw std::runtime_error("headless FPCR policy requires arm64");
#endif
}
void writeControl(std::uint64_t value) {
#if defined(__aarch64__)
    asm volatile("msr fpcr, %0\n\tisb" : : "r"(value) : "memory");
#else
    (void)value;
    throw std::runtime_error("headless FPCR policy requires arm64");
#endif
}
constexpr std::uint64_t FZ = 1ULL << 24;
}
FloatingPointMode::FloatingPointMode(bool enabled) : mPrevious(readControl()) {
    writeControl(enabled ? mPrevious | FZ : mPrevious & ~FZ);
}
FloatingPointMode::~FloatingPointMode() {
#if defined(__aarch64__)
    writeControl(mPrevious);
#endif
}
bool FloatingPointMode::flushToZero() { return (readControl() & FZ) != 0; }
}
