#pragma once

#include <revolution/types.h>

#ifdef GALAXY_NATIVE
namespace JMath {
    // Integer result of fctiwz with invalid-operation traps disabled. Narrow stores
    // retain low bits after this signed-word conversion, including on overflow.
    inline s32 truncToS32(f32 value) {
        if (value != value || value < -2147483648.0f) {
            return -2147483647 - 1;
        }
        if (value >= 2147483648.0f) {
            return 2147483647;
        }
        return static_cast<s32>(value);
    }

    inline u8 truncToU8(f32 value) {
        return static_cast<u8>(truncToS32(value));
    }

    inline u16 truncToU16(f32 value) {
        return static_cast<u16>(truncToS32(value));
    }

    inline s16 truncToS16(f32 value) {
        const u16 bits = truncToU16(value);
        const s32 signedValue = bits < 0x8000 ? bits : static_cast<s32>(bits) - 0x10000;
        return static_cast<s16>(signedValue);
    }
}  // namespace JMath
#endif
