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

void LINTLshift(u32* result, u32* input, u32 bits) {
    u32 count = *input;
    u32* digits = input + 1;
    u32* output = result + 1;
    u32 whole = bits / 32;
    u32 partial = bits % 32;
    u64 carry;
    u32 i;
    for (i = 0; i < whole; i++) output[i] = 0;
    carry = 0;
    for (i = 0; i < count; i++) {
        carry += (u64)digits[i] << partial;
        output[i + whole] = carry & 0xFFFFFFFF;
        carry = (carry >> 32) & 0xFFFFFFFF;
    }
    output[i + whole] = carry;
    *result = count + whole;
    if (carry) ++*result;
}

u32 LINTMsb(const u32* input) {
    u32 i;
    u32 count = *input;
    u32 high = input[count];
    u32 bits = 32;
    i = 0;
    for (; i < 32; i++, bits--) {
        if ((0x80000000u >> i) & high) break;
    }
    return (count - 1) * 32 + bits;
}

void LINTSub(u32* result, const u32* left, const u32* right) {
    const u32* leftDigits;
    u32* digit;
    const u32* rightDigits;
    u32 i;
    u32 count;
    u32 otherCount;
    u32* output;
    u64 borrow;
    count = *left;
    output = result + 1;
    digit = output;
    leftDigits = left + 1;
    rightDigits = right + 1;
    otherCount = *right;
    borrow = 0;
    for (i = 0; i < count; i++) {
        *digit = *leftDigits - (u32)borrow;
        borrow = *digit > 0xFFFFFFFFULL - borrow;
        if (i < otherCount) {
            *digit -= *rightDigits;
        }
        if (*digit > 0xFFFFFFFF - *rightDigits) borrow++;
        leftDigits++;
        digit++;
        rightDigits++;
    }
    digit = output + count;
    // The SDK probes the next limb before walking back to the highest nonzero limb.
    while (count && !*digit) {
        digit--;
        count--;
    }

    *result = count + 1;
}

// NONMATCHING: carry-register allocation and scheduling differ.
void LINTMul(u32* result, u32* left, u32* right) {
    u32 count = *left;
    u32 otherCount = *right;
    u32* a;
    u32* b;
    u32* output = result + 1;
    u64 carry = 0;
    u32 i, j;
    a = left + 1;
    b = right + 1;
    *result = 1;
    memset(output, 0, 128);
    if ((left[0] == 1 && left[1] == 0) || (right[0] == 1 && right[1] == 0)) return;
    for (i = 0; i < otherCount; i++) {
        carry = 0;
        for (j = 0; j < count; j++) {
            u64 sum = carry + output[i + j];
            sum += (u64)a[j] * b[i];
            output[i + j] = sum & 0xFFFFFFFF;
            carry = (sum >> 32) & 0xFFFFFFFF;
        }
        output[i + j] = carry;
    }
    if (!carry) *result = count + otherCount - 1;
    else *result = count + otherCount;
}
