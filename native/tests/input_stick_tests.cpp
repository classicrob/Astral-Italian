// Decode the production encoder through the ACTUAL SDK circle clamp, not a
// duplicate decoder. No game, cache, device or per-Rig KPAD initialization.
#include "../input/remote_model.hpp"
#include <revolution/kpad.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <initializer_list>
#include <stdexcept>

// Internal SDK helpers, compiled as C++ in the native build. Test-local only.
void clamp_stick_circle(Vec2*, s32, s32, s32, s32);
extern BOOL kp_stick_clamp_cross;
extern "C" void __OSThreadInit(void);
namespace In = PetariNative::Input;
namespace Det = PetariNative::Input::Detail;

namespace {
constexpr double pi = 3.14159265358979323846;
// Each rounded raw coordinate has error <= 1/2, so ||error|| <= sqrt(2)/2.
// The SDK radial map has slope 1/(71-15), capped at 0 and 1:
// length error <= sqrt(2)/(2*56) = sqrt(2)/112.
// The largest angle subtended by that error disk is asin(||error||/R),
// R=15+56*intended_length. At full scale: asin(sqrt(2)/142).
const double rawError = std::sqrt(2.0) / 2.0;
const double lengthError = rawError / (Det::kStickMax - Det::kStickMin);
// SDK output and target are float. This only bounds float arithmetic roundoff,
// separately from the derived integer quantization bound above.
constexpr double floatRoundoff = 8 * std::numeric_limits<float>::epsilon();
unsigned cases;
void check(bool ok, const char* why) { if (!ok) throw std::runtime_error(why); }
Vec2 decode(Det::RawStick raw) {
    Vec2 value{};
    clamp_stick_circle(&value, raw.x, raw.y, Det::kStickMin, Det::kStickMax);
    return value;
}
int legacyAxis(float value) {
    if (value == 0) return 0;
    const int raw = static_cast<int>(std::lround(15 + std::min(std::fabs(value), 1.0f) * 56));
    return value < 0 ? -raw : raw;
}
void verify(float x, float y) {
    ++cases;
    const auto raw = Det::rawStickVector(x, y);
    const auto decoded = decode(raw);
    const double inputLength = std::hypot(static_cast<double>(x), static_cast<double>(y));
    const double targetLength = std::min(inputLength, 1.0);
    const double actualLength = std::hypot(decoded.x, decoded.y);
    check(std::fabs(actualLength - targetLength) <= lengthError + floatRoundoff, "SDK decoded stick length exceeds derived quantization bound");
    if (actualLength > 0 && inputLength > 0) {
        const double angle = std::atan2(std::fabs(x * static_cast<double>(decoded.y) - y * static_cast<double>(decoded.x)),
                                       x * static_cast<double>(decoded.x) + y * static_cast<double>(decoded.y));
        const double radius = Det::kStickMin + (Det::kStickMax - Det::kStickMin) * targetLength;
        check(angle <= std::asin(rawError / radius) + floatRoundoff, "SDK decoded stick direction exceeds derived angular bound");
    } else {
        // A tiny intended vector can round inside the SDK's neutral disk;
        // its length error must still be bounded, but neutral has no direction.
        check(targetLength <= lengthError + floatRoundoff, "non-tiny vector became neutral");
    }
    const auto reflected = Det::rawStickVector(-x, -y);
    check(reflected.x == -raw.x && reflected.y == -raw.y, "raw vector sign symmetry");
}
void preservedBytes() {
    for (int i = -1000; i <= 1000; ++i) {
        const float value = i / 1000.0f;
        const auto x = Det::rawStickVector(value, 0);
        const auto y = Det::rawStickVector(0, value);
        check(x.x == legacyAxis(value) && x.y == 0 && y.x == 0 && y.y == legacyAxis(value), "axis bytes changed");
        verify(value, 0); verify(0, value);
    }
    for (int sx : {-1, 1}) for (int sy : {-1, 1}) {
        const auto raw = Det::rawStickVector(sx * 0.70710678f, sy * 0.70710678f);
        check(raw.x == sx * 55 && raw.y == sy * 55, "full keyboard diagonal bytes changed");
        verify(sx * 0.70710678f, sy * 0.70710678f);
    }
}
void producerReports() {
    for (bool walking : {false, true}) {
        Det::RemoteModel model;
        model.keyEvent(In::Key::W, true, false);
        model.keyEvent(In::Key::D, true, false);
        if (walking) model.keyEvent(In::Key::LeftAlt, true, false);
        const auto report = model.nextReport({});
        const auto decoded = decode({report.stickX, report.stickY});
        const double expected = walking ? 0.5 : 1.0;
        check(std::fabs(std::hypot(decoded.x, decoded.y) - expected) <= lengthError + floatRoundoff, "keyboard producer report has wrong SDK length");
        check(report.stickX == (walking ? 30 : 55) && report.stickY == (walking ? 30 : 55), "keyboard producer did not use the vector encoder");
    }
    Det::RemoteModel pad;
    In::Settings settings;
    settings.padStickDeadZone = 0;
    pad.setSettings(settings);
    pad.padAxisEvent(In::PadAxis::LeftX, static_cast<float>(std::cos(pi / 6)));
    pad.padAxisEvent(In::PadAxis::LeftY, -0.5f); // SDL y down.
    const auto report = pad.nextReport({});
    const auto decoded = decode({report.stickX, report.stickY});
    const double angle = std::atan2(decoded.y, decoded.x);
    check(std::fabs(angle - pi / 6) <= std::asin(rawError / Det::kStickMax) + floatRoundoff,
          "analog producer retains old 30-degree direction bias");
}
}
int main() try {
    __OSThreadInit();
    check(kp_stick_clamp_cross == FALSE, "SDK clamp mode changed");
    const auto invalid = Det::rawStickVector(std::numeric_limits<float>::quiet_NaN(), 1);
    check(invalid.x == 0 && invalid.y == 0, "non-finite input must not reach integer conversion");
    preservedBytes();
    for (double length : {0.0, 0.001, 0.01, 0.1, 0.25, 0.5, 0.75, 0.99, 1.0}) {
        for (int degree = 0; degree < 360; ++degree) {
            const double angle = degree * pi / 180;
            verify(static_cast<float>(length * std::cos(angle)), static_cast<float>(length * std::sin(angle)));
        }
    }
    producerReports();
    check(kp_stick_clamp_cross == FALSE, "test changed SDK clamp mode");
    std::printf("native radial stick: %u SDK-decoded vectors + producer/byte-preservation checks PASS\n", cases);
    return 0;
} catch (const std::exception& error) {
    std::fprintf(stderr, "FAIL: %s\n", error.what());
    return 1;
}
