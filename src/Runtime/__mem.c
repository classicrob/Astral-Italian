#include "__mem.h"

#pragma ANSI_strict off

// The original runtime copy is hand-written PowerPC assembly: byte fallbacks,
// aligned integer/floating-point transfers, and a cache prefetch. r3 retains dst.
#if defined(__MWERKS__) && !defined(GALAXY_NATIVE)
asm void* memcpy(void* pDest, const void* pSrc, size_t len) {
    nofralloc
    cmplwi cr1, r5, 0x0
    beqlr cr1
    cmplw cr1, r4, r3
    blt cr1, copyBackward
    beqlr cr1
    li r6, 0x80
    cmplw cr5, r5, r6
    blt cr5, forwardWords
    clrlwi r9, r4, 29
    clrlwi r10, r3, 29
    subf r8, r10, r3
    dcbt r0, r4
    xor. r11, r10, r9
    bne forwardBytes
    andi. r10, r10, 0x7
    beq+ forwardDoubles
    li r6, 0x8
    subf r9, r9, r6
    addi r8, r3, 0x0
    mtctr r9
    subf r5, r9, r5
alignForwardDoubles:
    lbz r9, 0x0(r4)
    addi r4, r4, 0x1
    stb r9, 0x0(r8)
    addi r8, r8, 0x1
    bdnz alignForwardDoubles
forwardDoubles:
    srwi r6, r5, 5
    mtctr r6
forwardDoubleLoop:
    lfd f1, 0x0(r4)
    lfd f2, 0x8(r4)
    lfd f3, 0x10(r4)
    lfd f4, 0x18(r4)
    addi r4, r4, 0x20
    stfd f1, 0x0(r8)
    stfd f2, 0x8(r8)
    stfd f3, 0x10(r8)
    stfd f4, 0x18(r8)
    addi r8, r8, 0x20
    bdnz forwardDoubleLoop
    andi. r6, r5, 0x1f
    beqlr
    subi r4, r4, 0x1
    mtctr r6
    subi r8, r8, 0x1
forwardDoubleTail:
    lbzu r9, 0x1(r4)
    stbu r9, 0x1(r8)
    bdnz forwardDoubleTail
    blr
forwardWords:
    li r6, 0x14
    cmplw cr5, r5, r6
    ble cr5, forwardBytes
    clrlwi r9, r4, 30
    clrlwi r10, r3, 30
    xor. r11, r10, r9
    bne forwardBytes
    li r6, 0x4
    subf r9, r9, r6
    addi r8, r3, 0x0
    subf r5, r9, r5
    mtctr r9
alignForwardWords:
    lbz r9, 0x0(r4)
    addi r4, r4, 0x1
    stb r9, 0x0(r8)
    addi r8, r8, 0x1
    bdnz alignForwardWords
    srwi r6, r5, 4
    mtctr r6
forwardWordLoop:
    lwz r9, 0x0(r4)
    lwz r10, 0x4(r4)
    lwz r11, 0x8(r4)
    lwz r12, 0xc(r4)
    addi r4, r4, 0x10
    stw r9, 0x0(r8)
    stw r10, 0x4(r8)
    stw r11, 0x8(r8)
    stw r12, 0xc(r8)
    addi r8, r8, 0x10
    bdnz forwardWordLoop
    andi. r6, r5, 0xf
    beqlr
    subi r4, r4, 0x1
    mtctr r6
    subi r8, r8, 0x1
forwardWordTail:
    lbzu r9, 0x1(r4)
    stbu r9, 0x1(r8)
    bdnz forwardWordTail
    blr
forwardBytes:
    subi r7, r4, 0x1
    subi r8, r3, 0x1
    mtctr r5
forwardByteLoop:
    lbzu r9, 0x1(r7)
    stbu r9, 0x1(r8)
    bdnz forwardByteLoop
    blr
copyBackward:
    add r4, r4, r5
    add r12, r3, r5
    li r6, 0x80
    cmplw cr5, r5, r6
    blt cr5, backwardWords
    clrlwi r9, r4, 29
    clrlwi r10, r12, 29
    xor. r11, r10, r9
    bne backwardBytes
    andi. r10, r10, 0x7
    beq+ backwardDoubles
    mtctr r10
alignBackwardDoubles:
    lbzu r9, -0x1(r4)
    stbu r9, -0x1(r12)
    bdnz alignBackwardDoubles
backwardDoubles:
    subf r5, r10, r5
    srwi r6, r5, 5
    mtctr r6
backwardDoubleLoop:
    lfd f1, -0x8(r4)
    lfd f2, -0x10(r4)
    lfd f3, -0x18(r4)
    lfd f4, -0x20(r4)
    subi r4, r4, 0x20
    stfd f1, -0x8(r12)
    stfd f2, -0x10(r12)
    stfd f3, -0x18(r12)
    stfdu f4, -0x20(r12)
    bdnz backwardDoubleLoop
    andi. r6, r5, 0x1f
    beqlr
    mtctr r6
backwardDoubleTail:
    lbzu r9, -0x1(r4)
    stbu r9, -0x1(r12)
    bdnz backwardDoubleTail
    blr
backwardWords:
    li r6, 0x14
    cmplw cr5, r5, r6
    ble cr5, backwardBytes
    clrlwi r9, r4, 30
    clrlwi r10, r12, 30
    xor. r11, r10, r9
    bne backwardBytes
    andi. r10, r10, 0x7
    beq+ backwardAlignedWords
    mtctr r10
alignBackwardWords:
    lbzu r9, -0x1(r4)
    stbu r9, -0x1(r12)
    bdnz alignBackwardWords
backwardAlignedWords:
    subf r5, r10, r5
    srwi r6, r5, 4
    mtctr r6
backwardWordLoop:
    lwz r9, -0x4(r4)
    lwz r10, -0x8(r4)
    lwz r11, -0xc(r4)
    lwz r8, -0x10(r4)
    subi r4, r4, 0x10
    stw r9, -0x4(r12)
    stw r10, -0x8(r12)
    stw r11, -0xc(r12)
    stwu r8, -0x10(r12)
    bdnz backwardWordLoop
    andi. r6, r5, 0xf
    beqlr
    mtctr r6
backwardWordTail:
    lbzu r9, -0x1(r4)
    stbu r9, -0x1(r12)
    bdnz backwardWordTail
    blr
backwardBytes:
    mtctr r5
backwardByteLoop:
    lbzu r9, -0x1(r4)
    stbu r9, -0x1(r12)
    bdnz backwardByteLoop
    blr
}
#else
void* memcpy(void *pDest, const void *pSrc, size_t len) {
    const char *p;
    char* q;
    int rev = pDest < pSrc;

    if (!rev) {
        for (p = (const char*)pSrc - 1, q = (char*)pDest - 1, len++; --len;) {
            *++q = *++p;
        }
    }
    else {
        for (p = (const char*)pSrc + len, q = (char*)pDest + len, len++; --len;) {
            *--q = *--p;
        }
    }

    return pDest;
}
#endif

void __fill_mem(void *pDest, int val, unsigned long n) {
	unsigned long v = (unsigned char) val;
	unsigned long i;

	((unsigned char*)pDest) = ((unsigned char*)pDest) - 1;

	if (n >= 32) {
		i = (~(unsigned long)pDest) & 3;

		if (i) {
			n -= i;

			do {
				*++(((unsigned char*)pDest)) = v;
			}
			while (--i);
		}

		if (v != 0) {
			v |= v << 24 | v << 16 | v <<  8;
		}

		((unsigned long*)pDest) = ((unsigned long*) (((unsigned char*)pDest) + 1)) - 1;
		i = n >> 5;

		if (i != 0) {
			do {
				*++(((unsigned long*)pDest)) = v;
				*++(((unsigned long*)pDest)) = v;
				*++(((unsigned long*)pDest)) = v;
				*++(((unsigned long*)pDest)) = v;
				*++(((unsigned long*)pDest)) = v;
				*++(((unsigned long*)pDest)) = v;
				*++(((unsigned long*)pDest)) = v;
				*++(((unsigned long*)pDest)) = v;
			}
			while (--i);
		}

		i = (n & 31) >> 2;

		if (i) {
			do {
				*++(((unsigned long*)pDest)) = v;
			}	
			while (--i);
		}

		((unsigned char*)pDest) = ((unsigned char*) (((unsigned long*)pDest) + 1)) - 1;
		n &= 3;
	}

	if (n) {
		do {
			*++(((unsigned char*)pDest)) = v;
		}
		while (--n);
	}
}

void* memset(void *pDest, int val, size_t num) {
    __fill_mem(pDest, val, num);
    return pDest;
}
