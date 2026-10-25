#ifndef RVL_SDK_WPAD_LINT_H
#define RVL_SDK_WPAD_LINT_H

#include "revolution/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Unsigned integers: word zero is the used limb count; little-endian u32 limbs
// follow it. Unused operand limbs and the result's next limb must be zero.
// Shift and multiply only read their operands and require distinct output storage.
// Subtraction permits result == left; left must be at least right.
s32 LINTCmp(const u32* left, const u32* right);
void LINTLshift(u32* result, u32* input, u32 bits);
u32 LINTMsb(const u32* input);
void LINTSub(u32* result, const u32* left, const u32* right);
void LINTMul(u32* result, u32* left, u32* right);

#ifdef __cplusplus
}
#endif

#endif
