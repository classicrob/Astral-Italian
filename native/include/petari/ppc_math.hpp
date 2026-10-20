// Floating-point result behavior derived from Dolphin's PowerPC interpreter.
// Copyright 2009 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <petari/ppc_float.hpp>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace PetariNative::PPC {
inline std::uint64_t doubleBits(double value) noexcept {
    std::uint64_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

inline double doubleFromBits(std::uint64_t bits) noexcept {
    double value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

inline double quietNaN(double value) noexcept {
    return doubleFromBits(doubleBits(value) | 0x0008000000000000ULL);
}

inline double canonicalNaN() noexcept {
    return doubleFromBits(0x7ff8000000000000ULL);
}

// lfs and an unquantized psq_l preserve the single's NaN payload and sign.
inline double loadSingle(float value) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    const std::uint64_t sign = std::uint64_t(bits & 0x80000000U) << 32;
    std::uint32_t fraction = bits & 0x007fffffU;
    const unsigned exponent = (bits >> 23) & 255;
    if (exponent == 255)
        return doubleFromBits(sign | 0x7ff0000000000000ULL | (std::uint64_t(fraction) << 29));
    if (exponent != 0)
        return doubleFromBits(sign | (std::uint64_t(exponent + 896) << 52) | (std::uint64_t(fraction) << 29));
    if (fraction == 0)
        return doubleFromBits(sign);
    int unbiased = -126;
    while ((fraction & 0x00800000U) == 0) {
        fraction <<= 1;
        --unbiased;
    }
    return doubleFromBits(sign | (std::uint64_t(unbiased + 1023) << 52) |
                          (std::uint64_t(fraction & 0x007fffffU) << 29));
}

// psq_st with GQR type 0 flushes single subnormals even when FPSCR.NI is clear.
inline float storePaired(double value) noexcept {
    const auto bits = doubleBits(value);
    const unsigned exponent = unsigned((bits >> 52) & 0x7ff);
    std::uint32_t result = std::uint32_t((bits >> 32) & 0x80000000U);
    if (exponent > 896 || (bits & 0x7fffffffffffffffULL) == 0)
        result = std::uint32_t(((bits >> 32) & 0xc0000000U) | ((bits >> 29) & 0x3fffffffU));
    float output;
    std::memcpy(&output, &result, sizeof(output));
    return output;
}

inline double addSingle(double a, double b) noexcept {
    if (std::isnan(a)) return loadSingle(forceSingle(quietNaN(a)));
    if (std::isnan(b)) return loadSingle(forceSingle(quietNaN(b)));
    const double value = a + b;
    return loadSingle(forceSingle(std::isnan(value) ? canonicalNaN() : value));
}

inline double subtractSingle(double a, double b) noexcept {
    if (std::isnan(a)) return loadSingle(forceSingle(quietNaN(a)));
    if (std::isnan(b)) return loadSingle(forceSingle(quietNaN(b)));
    const double value = a - b;
    return loadSingle(forceSingle(std::isnan(value) ? canonicalNaN() : value));
}

inline double multiplySingle(double a, double c) noexcept {
    const double multiplier = force25Bit(c);
    if (std::isnan(a)) return loadSingle(forceSingle(quietNaN(a)));
    if (std::isnan(multiplier)) return loadSingle(forceSingle(quietNaN(multiplier)));
    const double value = a * multiplier;
    return loadSingle(forceSingle(std::isnan(value) ? canonicalNaN() : value));
}

// PowerPC checks NaN operands in a,b,c order. The product's c operand is rounded
// by the Gekko multiplier rule; a double FMA halfway result needs its residual before
// the final single rounding. This also handles an unrounded estimate operand.
inline double multiplyAddSingle(double a, double c, double b, bool subtract = false) noexcept {
    const double multiplier = force25Bit(c);
    const double addend = subtract ? -b : b;
    double value = std::fma(a, multiplier, addend);
    auto bits = doubleBits(value);
    if ((bits & 0x000000001fffffffULL) == 0x0000000010000000ULL) {
        const double aPrime = addend - value;
        const double bPrime = value + aPrime;
        const double deltaA = std::fma(a, multiplier, aPrime);
        const double deltaB = addend - bPrime;
        const double error = deltaA + deltaB;
        if (error != 0.0) {
            bits = (error > 0.0) == (value > 0.0) ? bits + 1 : bits - 1;
            value = doubleFromBits(bits);
        }
    }
    if (std::isnan(value)) {
        if (std::isnan(a)) return loadSingle(forceSingle(quietNaN(a)));
        if (std::isnan(b)) return loadSingle(forceSingle(quietNaN(b)));
        if (std::isnan(c)) return loadSingle(forceSingle(quietNaN(c)));
        value = canonicalNaN();
    }
    return loadSingle(forceSingle(value));
}

inline double negativeMultiplySubtractSingle(double a, double c, double b) noexcept {
    const double value = multiplyAddSingle(a, c, b, true);
    return std::isnan(value) ? value : -value;
}

inline double negativeMultiplyAddSingle(double a, double c, double b) noexcept {
    const double value = multiplyAddSingle(a, c, b);
    return std::isnan(value) ? value : -value;
}

struct Paired {
    double first;
    double second;
};

inline Paired loadPair(const float* value, bool one = false) noexcept {
    float lanes[2];
    std::memcpy(lanes, value, one ? sizeof(float) : sizeof(lanes));
    return {loadSingle(lanes[0]), one ? 1.0 : loadSingle(lanes[1])};
}

inline Paired splat(double value) noexcept { return {value, value}; }
inline Paired add(Paired a, Paired b) noexcept {
    return {addSingle(a.first, b.first), addSingle(a.second, b.second)};
}
inline Paired subtract(Paired a, Paired b) noexcept {
    return {subtractSingle(a.first, b.first), subtractSingle(a.second, b.second)};
}
inline Paired multiply(Paired a, Paired c) noexcept {
    return {multiplySingle(a.first, c.first), multiplySingle(a.second, c.second)};
}
inline Paired multiplyAdd(Paired a, Paired c, Paired b) noexcept {
    return {multiplyAddSingle(a.first, c.first, b.first), multiplyAddSingle(a.second, c.second, b.second)};
}
inline Paired multiplySubtract(Paired a, Paired c, Paired b) noexcept {
    return {multiplyAddSingle(a.first, c.first, b.first, true), multiplyAddSingle(a.second, c.second, b.second, true)};
}
inline Paired negativeMultiplySubtract(Paired a, Paired c, Paired b) noexcept {
    return {negativeMultiplySubtractSingle(a.first, c.first, b.first),
            negativeMultiplySubtractSingle(a.second, c.second, b.second)};
}
inline Paired negativeMultiplyAdd(Paired a, Paired c, Paired b) noexcept {
    return {negativeMultiplyAddSingle(a.first, c.first, b.first),
            negativeMultiplyAddSingle(a.second, c.second, b.second)};
}
inline Paired multiplyScalar0(Paired a, Paired c) noexcept {
    return {multiplySingle(a.first, c.first), multiplySingle(a.second, c.first)};
}
inline Paired multiplyScalar1(Paired a, Paired c) noexcept {
    return {multiplySingle(a.first, c.second), multiplySingle(a.second, c.second)};
}
inline Paired multiplyAddScalar0(Paired a, Paired c, Paired b) noexcept {
    return {multiplyAddSingle(a.first, c.first, b.first), multiplyAddSingle(a.second, c.first, b.second)};
}
inline Paired multiplyAddScalar1(Paired a, Paired c, Paired b) noexcept {
    return {multiplyAddSingle(a.first, c.second, b.first), multiplyAddSingle(a.second, c.second, b.second)};
}
inline Paired sum0(Paired a, Paired c, Paired b) noexcept {
    return {addSingle(a.first, b.second), loadSingle(forceSingle(c.second))};
}
inline Paired sum1(Paired a, Paired c, Paired b) noexcept {
    return {loadSingle(forceSingle(c.first)), addSingle(a.first, b.second)};
}
inline Paired negate(Paired a) noexcept { return {-a.first, -a.second}; }
inline Paired merge00(Paired a, Paired b) noexcept { return {a.first, b.first}; }
inline Paired merge01(Paired a, Paired b) noexcept { return {a.first, b.second}; }
inline Paired merge10(Paired a, Paired b) noexcept { return {a.second, b.first}; }
inline Paired merge11(Paired a, Paired b) noexcept { return {a.second, b.second}; }
inline void storePair(float* out, Paired value, bool one = false) noexcept {
    const float lanes[2] = {storePaired(value.first), storePaired(value.second)};
    std::memcpy(out, lanes, one ? sizeof(float) : sizeof(lanes));
}
}  // namespace PetariNative::PPC
