// Estimate tables and rounding contracts adapted from Dolphin Emulator Project.
// Copyright 2009, 2018 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later
// Reference revision: cf5cb7ef81c3e8e7af5c9d566334219ffda9f68b
// Source/Core/Common/FloatUtils.cpp and Core/PowerPC/Interpreter/Interpreter_FPUtils.h
#include <revolution/types.h>
#include <petari/ppc_float.hpp>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

namespace PetariNative {
namespace PPC {
namespace {
static_assert(sizeof(double) == 8 && std::numeric_limits<double>::is_iec559 && std::numeric_limits<double>::digits == 53);
static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559 && std::numeric_limits<float>::digits == 24);

constexpr std::uint64_t signMask = 0x8000000000000000ULL;
constexpr std::uint64_t fractionMask = 0x000FFFFFFFFFFFFFULL;
constexpr std::uint64_t exponentMask = 0x7FF0000000000000ULL;
constexpr std::uint64_t quietBit = 0x0008000000000000ULL;

std::uint64_t wordOf(double value) noexcept {
    std::uint64_t word;
    std::memcpy(&word, &value, sizeof(word));
    return word;
}

double doubleOf(std::uint64_t word) noexcept {
    double value;
    std::memcpy(&value, &word, sizeof(value));
    return value;
}

float singleOf(std::uint32_t word) noexcept {
    float value;
    std::memcpy(&value, &word, sizeof(value));
    return value;
}

struct EstimateEntry {
    std::uint32_t base;
    std::uint32_t decrement;
};

constexpr EstimateEntry squareRootTable[32] = {
    {0x1A7E800U, 0x568U},
    {0x17CB800U, 0x4F3U},
    {0x1552800U, 0x48DU},
    {0x130C000U, 0x435U},
    {0x10F2000U, 0x3E7U},
    {0x0EFF000U, 0x3A2U},
    {0x0D2E000U, 0x365U},
    {0x0B7C000U, 0x32EU},
    {0x09E5000U, 0x2FCU},
    {0x0867000U, 0x2D0U},
    {0x06FF000U, 0x2A8U},
    {0x05AB800U, 0x283U},
    {0x046A000U, 0x261U},
    {0x0339800U, 0x243U},
    {0x0218800U, 0x226U},
    {0x0105800U, 0x20BU},
    {0x3FFA000U, 0x7A4U},
    {0x3C29000U, 0x700U},
    {0x38AA000U, 0x670U},
    {0x3572000U, 0x5F2U},
    {0x3279000U, 0x584U},
    {0x2FB7000U, 0x524U},
    {0x2D26000U, 0x4CCU},
    {0x2AC0000U, 0x47EU},
    {0x2881000U, 0x43AU},
    {0x2665000U, 0x3FAU},
    {0x2468000U, 0x3C2U},
    {0x2287000U, 0x38EU},
    {0x20C1000U, 0x35EU},
    {0x1F12000U, 0x332U},
    {0x1D79000U, 0x30AU},
    {0x1BF4000U, 0x2E6U},
};

constexpr EstimateEntry reciprocalTable[32] = {
    {0x7FF800U, 0x3E1U},
    {0x783800U, 0x3A7U},
    {0x70EA00U, 0x371U},
    {0x6A0800U, 0x340U},
    {0x638800U, 0x313U},
    {0x5D6200U, 0x2EAU},
    {0x579000U, 0x2C4U},
    {0x520800U, 0x2A0U},
    {0x4CC800U, 0x27FU},
    {0x47CA00U, 0x261U},
    {0x430800U, 0x245U},
    {0x3E8000U, 0x22AU},
    {0x3A2C00U, 0x212U},
    {0x360800U, 0x1FBU},
    {0x321400U, 0x1E5U},
    {0x2E4A00U, 0x1D1U},
    {0x2AA800U, 0x1BEU},
    {0x272C00U, 0x1ACU},
    {0x23D600U, 0x19BU},
    {0x209E00U, 0x18BU},
    {0x1D8800U, 0x17CU},
    {0x1A9000U, 0x16EU},
    {0x17AE00U, 0x15BU},
    {0x14F800U, 0x15BU},
    {0x124400U, 0x143U},
    {0x0FBE00U, 0x143U},
    {0x0D3800U, 0x12DU},
    {0x0ADE00U, 0x12DU},
    {0x088400U, 0x11AU},
    {0x065000U, 0x11AU},
    {0x041C00U, 0x108U},
    {0x020C00U, 0x106U},
};

std::uint64_t roundRightEven(std::uint64_t value, unsigned shift) noexcept {
    const std::uint64_t retained = value >> shift;
    const std::uint64_t discarded = value & ((std::uint64_t(1) << shift) - 1);
    const std::uint64_t halfway = std::uint64_t(1) << (shift - 1);
    return retained + (discarded > halfway || (discarded == halfway && (retained & 1)));
}
}  // namespace

double frsqrte(double value) noexcept {
    const std::uint64_t word = wordOf(value);
    const std::uint64_t sign = word & signMask;
    std::uint64_t fraction = word & fractionMask;
    int exponent = static_cast<int>((word >> 52) & 0x7FF);
    if (exponent == 0 && fraction == 0) {
        return doubleOf(sign | exponentMask);
    }
    if (exponent == 0x7FF) {
        if (fraction != 0) return doubleOf(word | quietBit);
        return doubleOf(sign ? exponentMask | quietBit : 0);
    }
    if (sign != 0) return doubleOf(exponentMask | quietBit);
    if (exponent == 0) {
        do {
            fraction <<= 1;
            --exponent;
        } while ((fraction & (std::uint64_t(1) << 52)) == 0);
        fraction &= fractionMask;
        ++exponent;
    }
    const unsigned parity = static_cast<unsigned>(exponent) & 1;
    const unsigned segment = (parity << 4) | static_cast<unsigned>(fraction >> 48);
    const unsigned position = static_cast<unsigned>((fraction >> 37) & 0x7FF);
    const EstimateEntry& entry = squareRootTable[segment];
    const std::uint64_t estimate = entry.base - entry.decrement * position;
    const unsigned resultExponent = static_cast<unsigned>((3068 - exponent) / 2);
    return doubleOf((std::uint64_t(resultExponent) << 52) | (estimate << 26));
}

double fres(double value) noexcept {
    const std::uint64_t word = wordOf(value);
    const std::uint64_t sign = word & signMask;
    const std::uint64_t fraction = word & fractionMask;
    const unsigned exponent = static_cast<unsigned>((word >> 52) & 0x7FF);
    if (exponent == 0 && fraction == 0) return doubleOf(sign | exponentMask);
    if (exponent == 0x7FF) {
        return doubleOf(fraction != 0 ? word | quietBit : sign);
    }
    if (exponent < 895) return doubleOf(sign | 0x47EFFFFFE0000000ULL);
    if (exponent >= 1149) return doubleOf(sign);
    const unsigned segment = static_cast<unsigned>(fraction >> 47);
    const unsigned position = static_cast<unsigned>((fraction >> 37) & 0x3FF);
    const EstimateEntry& entry = reciprocalTable[segment];
    const std::uint64_t estimate = entry.base - (entry.decrement * position + 1) / 2;
    return doubleOf(sign | (std::uint64_t(2045 - exponent) << 52) | (estimate << 29));
}

double force25Bit(double value) noexcept {
    const std::uint64_t word = wordOf(value);
    std::uint64_t keep = 0xFFFFFFFFF8000000ULL;
    std::uint64_t rounding = 0x08000000ULL;
    std::uint64_t fraction = word & fractionMask;
    if ((word & exponentMask) == 0 && fraction != 0) {
        unsigned shift = 0;
        while ((fraction & (std::uint64_t(1) << 52)) == 0) {
            fraction <<= 1;
            ++shift;
        }
        keep = ~(0x07FFFFFFULL >> shift);
        rounding >>= shift;
    }
    return doubleOf((word & keep) + (word & rounding));
}

bool nonIeeeMode() noexcept {
#if defined(__aarch64__)
    std::uint64_t control;
    asm volatile("mrs %0, fpcr" : "=r"(control));
    return (control & (std::uint64_t(1) << 24)) != 0;
#else
    return false;
#endif
}

float forceSingle(double value, bool nonIeee) noexcept {
    const std::uint64_t word = wordOf(value);
    const std::uint64_t magnitude = word & ~signMask;
    const std::uint32_t sign = static_cast<std::uint32_t>(word >> 32) & 0x80000000U;
    if (nonIeee && magnitude < 0x3810000000000000ULL) return singleOf(sign);
    const unsigned exponentBits = static_cast<unsigned>((word >> 52) & 0x7FF);
    const std::uint64_t fraction = word & fractionMask;
    if (exponentBits == 0x7FF) {
        if (fraction == 0) return singleOf(sign | 0x7F800000U);
        return singleOf(sign | 0x7FC00000U | static_cast<std::uint32_t>(fraction >> 29));
    }
    if (exponentBits == 0) return singleOf(sign);
    int exponent = static_cast<int>(exponentBits) - 1023;
    if (exponent > 127) return singleOf(sign | 0x7F800000U);
    if (exponent < -150) return singleOf(sign);
    const std::uint64_t significand = (std::uint64_t(1) << 52) | fraction;
    if (exponent < -126) {
        const unsigned shift = static_cast<unsigned>(-exponent - 97);
        return singleOf(sign | static_cast<std::uint32_t>(roundRightEven(significand, shift)));
    }
    std::uint64_t rounded = roundRightEven(significand, 29);
    if (rounded == (std::uint64_t(1) << 24)) {
        rounded >>= 1;
        ++exponent;
    }
    if (exponent > 127) return singleOf(sign | 0x7F800000U);
    return singleOf(sign | (static_cast<std::uint32_t>(exponent + 127) << 23) |
                    static_cast<std::uint32_t>(rounded & 0x7FFFFF));
}
}  // namespace PPC
}  // namespace PetariNative

f64 __frsqrte(f64 value) { return PetariNative::PPC::frsqrte(value); }
f64 __fres(f64 value) { return PetariNative::PPC::fres(value); }
u32 __cntlzw(u32 value) { return value == 0 ? 32 : __builtin_clz(value); }
s32 __abs(s32 value) { return value < 0 ? static_cast<s32>(0u - static_cast<u32>(value)) : value; }
f32 __fabsf(f32 value) { return std::fabs(value); }
f64 __fabs(f64 value) { return std::fabs(value); }
void* __memcpy(void* dst, const void* src, int size) { return std::memcpy(dst, src, size); }
