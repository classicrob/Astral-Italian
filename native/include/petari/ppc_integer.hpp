#pragma once

#include <cstdint>
#include <limits>

namespace PetariNative::PPC {

// Interpret the low word without an out-of-range unsigned-to-signed cast.
inline std::int32_t signedWord(std::uint32_t bits) {
    return bits <= 0x7fffffffu ? static_cast<std::int32_t>(bits)
                             : static_cast<std::int32_t>(std::int64_t(bits) - 0x100000000LL);
}

inline std::int32_t multiplyWord(std::int32_t left, std::int32_t right) {
    return signedWord(static_cast<std::uint32_t>(left) * static_cast<std::uint32_t>(right));
}

inline std::int32_t subtractWord(std::int32_t left, std::int32_t right) {
    return signedWord(static_cast<std::uint32_t>(left) - static_cast<std::uint32_t>(right));
}

// Gekko DIVW result as implemented by Dolphin's Interpreter::divwx. Invalid
// division leaves -1 for a negative numerator and 0 otherwise. No XER/CR flags
// are modeled here; the capture caller uses only the destination word.
inline std::int32_t divideSignedWord(std::int32_t numerator, std::int32_t denominator) {
    if (denominator == 0 ||
        (numerator == std::numeric_limits<std::int32_t>::min() && denominator == -1)) {
        return numerator < 0 ? -1 : 0;
    }
    return numerator / denominator;
}

}  // namespace PetariNative::PPC
