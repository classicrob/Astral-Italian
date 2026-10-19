#pragma once
#include <cstdint>
namespace PetariNative::Headless {
class FloatingPointMode {
public:
    explicit FloatingPointMode(bool flushToZero);
    ~FloatingPointMode();
    FloatingPointMode(const FloatingPointMode&) = delete;
    FloatingPointMode& operator=(const FloatingPointMode&) = delete;
    static bool flushToZero();
private:
    std::uint64_t mPrevious = 0;
};
}
