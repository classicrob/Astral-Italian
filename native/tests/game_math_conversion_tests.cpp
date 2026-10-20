// U066–072: actual MR conversion sites, linked from MathUtil.cpp/MtxUtil.cpp.
// Expected values follow the DOL's signed fctiwz followed by low-halfword use.
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include <JSystem/JMath/JMAFloatConversion.hpp>
#include <JSystem/JMath/JMATrigonometric.hpp>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>

namespace {
int checks;
void check(bool value, const char* message) {
    ++checks;
    if (!value) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}
struct Conversion { f32 value; s16 expected; };
const f32 inf = std::numeric_limits<f32>::infinity();
const f32 nan = std::numeric_limits<f32>::quiet_NaN();
const Conversion conversions[] = {
    {0.0f, 0}, {-0.0f, 0}, {1.75f, 1}, {-1.75f, -1},
    {32767.99f, 32767}, {32768.0f, -32768}, {-32769.0f, 32767},
    {65535.75f, -1}, {65536.75f, 0}, {-65537.75f, -1},
    {2147483520.0f, -128}, {2147483648.0f, -1}, {-2147483648.0f, 0},
    {std::nextafter(-2147483648.0f, -inf), 0},
    {std::numeric_limits<f32>::max(), -1}, {std::numeric_limits<f32>::lowest(), 0},
    {std::numeric_limits<f32>::denorm_min(), 0}, {inf, -1}, {-inf, 0}, {nan, 0}, {-nan, 0}
};
void testConversions() {
    for (const auto& test : conversions) {
        check(JMath::truncToS16(test.value) == test.expected, "canonical signed-word/low16 conversion");
        // Canaries prove these sites write three halfwords, not three words.
        struct Guarded { u16 before = 0x1234; TVec3s value; u16 after = 0xabcd; } result;
        const TVec3f source(test.value, test.value, test.value);
        MR::floatToFixed16(&result.value, source, 0);
        check(result.value.x == test.expected, "floatToFixed16 x");
        check(result.value.y == test.expected, "floatToFixed16 y");
        check(result.value.z == test.expected, "floatToFixed16 z");
        check(result.before == 0x1234 && result.after == 0xabcd, "halfword destination bounds");
    }
}
s16 low16(std::int64_t word) {
    const auto bits = static_cast<std::uint64_t>(word) & 0xffffu;
    return static_cast<s16>(bits < 0x8000 ? static_cast<std::int32_t>(bits)
                                        : static_cast<std::int32_t>(bits) - 0x10000);
}
void testAllShiftCounts() {
    // An independent integer oracle for SLW -> signed word scale. These input
    // fractions are exact, and their products stay inside signed-word range.
    for (unsigned q = 0; q < 256; ++q) {
        const unsigned count = q % 64;
        const std::int64_t word = count >= 32 ? 0 : count == 31 ? -2147483648LL : (1LL << count);
        const TVec3f source(1.0f, 0.5f, -0.25f);
        TVec3s actual;
        MR::floatToFixed16(&actual, source, static_cast<u8>(q));
        check(actual.x == low16(word), "SLW q: x");
        check(actual.y == low16(word / 2), "SLW q: y");
        check(actual.z == low16(-word / 4), "SLW q: z");
    }
    TVec3s actual;
    MR::floatToFixed16(&actual, TVec3f(-1.0f, 1.0f, inf), 31);
    check(actual.x == -1 && actual.y == 0 && actual.z == 0, "q31 scale is negative before conversion");
    MR::floatToFixed16(&actual, TVec3f(1.0f, inf, nan), 32);
    check(actual.x == 0 && actual.y == 0 && actual.z == 0, "zero scale: finite/Inf/NaN products");
}
struct Rotation { f32 degrees; s16 angle; };
const Rotation rotations[] = {
    {0.0f, 0}, {1.0f, 182}, {-1.0f, -182}, {180.0f, -32768}, {-180.0f, -32768},
    {360.0f, 0}, {-360.0f, 0}, {540.0f, -32768}, {inf, -1}, {-inf, 0}, {nan, 0},
    {1.0e20f, -1}, {-1.0e20f, 0}, {std::numeric_limits<f32>::max(), -1}
};
void referenceRotation(Mtx result, s16 x, s16 y, s16 z) {
    const double sx = JMASSin(x), cx = JMASCos(x);
    const double sy = JMASSin(y), cy = JMASCos(y);
    const double sz = JMASSin(z), cz = JMASCos(z);
    const double axes[3][3][3] = {
        {{1, 0, 0}, {0, cx, -sx}, {0, sx, cx}},
        {{cy, 0, sy}, {0, 1, 0}, {-sy, 0, cy}},
        {{cz, -sz, 0}, {sz, cz, 0}, {0, 0, 1}}
    };
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            double value = 0;
            for (int j = 0; j < 3; ++j)
                for (int k = 0; k < 3; ++k)
                    value += axes[2][row][j] * axes[1][j][k] * axes[0][k][col];
            result[row][col] = static_cast<f32>(value);
        }
        result[row][3] = 0;
    }
}
void checkMatrix(const Mtx actual, const Mtx expected) {
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 4; ++col)
            check(std::isfinite(actual[row][col]) &&
                  std::fabs(actual[row][col] - expected[row][col]) < 2e-6f, "rotation result");
}
void testRotations() {
    for (unsigned i = 0; i < sizeof(rotations) / sizeof(rotations[0]); ++i) {
        const auto& x = rotations[i];
        const auto& y = rotations[(i + 3) % (sizeof(rotations) / sizeof(rotations[0]))];
        const auto& z = rotations[(i + 7) % (sizeof(rotations) / sizeof(rotations[0]))];
        Mtx actual, expected;
        MR::makeMtxRotate(actual, x.degrees, y.degrees, z.degrees);
        referenceRotation(expected, x.angle, y.angle, z.angle);
        checkMatrix(actual, expected);
        MR::makeMtxRotateY(actual, x.degrees);
        referenceRotation(expected, 0, x.angle, 0);
        checkMatrix(actual, expected);
    }
}
}
int main() {
    testConversions(); testAllShiftCounts(); testRotations();
    std::printf("%d actual game math conversion checks passed\n", checks);
}
