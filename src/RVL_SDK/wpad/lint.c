#include "revolution/wpad/lint.h"
#include <cstring>

/* Little-endian 32-bit limbs, with the used limb count in element zero. */
s32 LINTCmp(const u32* left, const u32* right) {
    u32 count = *left++;
    u32 otherCount = *right++;
    s32 index;
    if (count > otherCount) return 1;
    if (count < otherCount) return -1;
    for (index = count - 1; index >= 0; index--) {
        if (left[index] > right[index]) return 1;
        if (left[index] < right[index]) return -1;
    }
    return 0;
}

// NONMATCHING: carry-zero placement and GPR allocation; same loop CFG and integer operations.
void LINTLshift(u32* result, const u32* input, u32 bits) {
    u32 count = *input++;
    u32* output = result + 1;
    u32 whole = bits / 32;
    u32 partial = bits % 32;
    u64 carry = 0;
    u32 i;
    for (i = 0; i < whole; i++) output[i] = 0;
    for (i = 0; i < count; i++) {
        carry += (u64)input[i] << partial;
        output[i + whole] = carry & 0xFFFFFFFF;
        carry = (carry >> 32) & 0xFFFFFFFF;
    }
    output[i + whole] = carry;
    *result = count + whole;
    if (carry) ++*result;
}

// NONMATCHING: GPR allocation and one independent counter-update order.
u32 LINTMsb(const u32* input) {
    u32 count = *input;
    u32 high = input[count];
    u32 bits = 32;
    u32 i;
    for (i = 0; i < 32; i++) {
        if ((0x80000000u >> i) & high) break;
        bits--;
    }
    return (count - 1) * 32 + bits;
}

// NONMATCHING: dead high-word subtraction, saved GPR range and loop-alignment NOP.
void LINTSub(u32* result, const u32* left, const u32* right) {
    u32 count = *left++;
    u32 otherCount = *right++;
    u32* output = result + 1;
    u64 borrow = 0;
    u32* digit = output;
    u32 i;
    for (i = 0; i < count; i++) {
        *digit = *left - borrow;
        borrow = *digit > 0xFFFFFFFFULL - borrow;
        if (i < otherCount) {
            *digit -= *right;
        }
        if (*digit > 0xFFFFFFFF - *right) borrow++;
        left++;
        digit++;
        right++;
    }
    digit = output + count;
    // The SDK probes the next limb before walking back to the highest nonzero limb.
    while (count && !*digit) {
        digit--;
        count--;
    }

    *result = count + 1;
}

// NONMATCHING: spills and carry-register allocation; repeated read-only operand loads are reused.
void LINTMul(u32* result, const u32* left, const u32* right) {
    u32 count = *left;
    u32 otherCount = *right;
    u32* output = result + 1;
    const u32* a = left + 1;
    const u32* b = right + 1;
    u64 carry = 0;
    u32 i, j;
    *result = 1;
    memset(output, 0, 128);
    if ((left[0] == 1 && left[1] == 0) || (right[0] == 1 && right[1] == 0)) return;
    for (i = 0; i < otherCount; i++) {
        carry = 0;
        for (j = 0; j < count; j++) {
            carry += output[i + j];
            carry += (u64)a[j] * b[i];
            output[i + j] = carry & 0xFFFFFFFF;
            carry = (carry >> 32) & 0xFFFFFFFF;
        }
        output[i + j] = carry;
    }
    if (!carry) *result = count + otherCount - 1;
    else *result = count + otherCount;
}
