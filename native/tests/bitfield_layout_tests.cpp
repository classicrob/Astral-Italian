// Regression checks for game flags that also have numeric-mask views. SMG2's
// movement/draw states are raw words; remaining bitfields must retain the Wii
// compiler's most-significant-bit ordering on the native little-endian build.
// Links src/Game/Player/RushEndInfo.cpp; other game headers are used header-only.
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/RushEndInfo.hpp"
#include <cstdio>
#include <cstring>

static int sFailures = 0;
static void check(bool condition, const char* text) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", text);
        ++sFailures;
    }
}

template < typename T >
static u32 word(const T& value, int index = 0) {
    u32 result;
    std::memcpy(&result, reinterpret_cast< const u8* >(&value) + index * 4, sizeof(result));
    return result;
}

static void testMovementStates() {
    static_assert(sizeof(Mario::MovementStates) == 8, "MovementStates is two words");
    Mario::MovementStates states;
    std::memset(&states, 0, sizeof(states));
    check(word(states, 0) == 0 && word(states, 1) == 0, "MovementStates zero-initializes");

    states.mLowWord = Mario::MovementStates::Jumping;
    check(word(states, 0) == 0x80000000 && word(states, 1) == 0, "Jumping is bit 31 of the low word");
    states.mLowWord = Mario::MovementStates::Turning | Mario::MovementStates::DebugMode;
    check(word(states, 0) == 0x10000200, "Turning and DebugMode masks retain their numeric view");
    states.mLowWord = Mario::MovementStates::DigitalJump;
    states.mHighWord = 0x80000000;
    check(word(states, 0) == 2 && word(states, 1) == 0x80000000, "movement words retain their order");
}

static void testDrawStates() {
    static_assert(sizeof(Mario::DrawStates) == 4, "DrawStates is one word");
    Mario::DrawStates states;
    std::memset(&states, 0, sizeof(states));

    states.mWord = Mario::DrawStates::Underwater;
    check(word(states) == 0x2000, "Underwater is bit 13");
    u32 value = 0x00102000;
    std::memcpy(&states, &value, sizeof(value));
    check((states.mWord & Mario::DrawStates::Underwater) != 0, "numeric draw state preserves Underwater");
}

static void testRushEndInfo() {
    RushEndInfo info(nullptr, 3, TVec3f(0.0f, 0.0f, 0.0f), true, 0);
    check(info._20 == 0, "RushEndInfo flags start at zero");

    // PlayerUtil sets `_20 |= 0xC0000000` and `mFlags.mDamageType = N`; MarioActorRush reads
    // (_20 >> 30) & 1, _20 >> 31 and (_20 >> 24) & 15.
    info._20 |= 0xC0000000;
    info.mFlags.mDamageType = 6;
    check(info._20 == 0xC6000000, "mDamageType occupies bits 27-24");
    check(((info._20 >> 24) & 15) == 6 && ((info._20 >> 30) & 1) == 1 && (info._20 >> 31) == 1, "numeric reads of the flag word");
    check(info.mFlags._0 == 0xC, "_0 occupies bits 31-28");

    info._20 = 0x00800000;  // PlayerUtil: info._20 |= 0x800000
    check(((info._20 >> 23) & 1) == 1 && info.mFlags._8 == 0x800000 && info.mFlags.mDamageType == 0, "_8 occupies bits 23-0");
}

static void testMarioEffectFlags() {
    typedef decltype(static_cast< MarioActor* >(nullptr)->mEffectFlags) EffectFlags;
    static_assert(sizeof(EffectFlags) == 4, "effect flags are one word");
    EffectFlags flags;
    std::memset(&flags, 0, sizeof(flags));
    flags.mSmoke = true;
    check(word(flags) == 0x80000000, "mSmoke is bit 31");
    flags.mSmoke = false;
    flags.mBeeWind = true;
    check(word(flags) == (1u << 26), "mBeeWind is bit 26");
}

int main() {
    testMovementStates();
    testDrawStates();
    testRushEndInfo();
    testMarioEffectFlags();

    if (sFailures != 0) {
        std::fprintf(stderr, "%d bitfield layout check(s) failed\n", sFailures);
        return 1;
    }
    std::puts("Bitfield layout tests passed");
    return 0;
}
