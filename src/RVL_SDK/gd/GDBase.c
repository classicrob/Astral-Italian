#include "revolution/gd/GDBase.h"
#ifdef GALAXY_NATIVE
#include "revolution/os/OSCache.h"
#include <stdint.h>
#endif

GDLObj* __GDCurrentDL = NULL;
static GDOverflowCallback overflowcb = NULL;

void GDInitGDLObj(GDLObj* pDL, void* pStart, u32 length) {
#ifdef GALAXY_NATIVE
    pDL->start = static_cast<u8*>(pStart);
#else
    pDL->start = pStart;
#endif
    pDL->ptr = (u8*)pStart;
    pDL->top = (u8*)pStart + length;
    pDL->length = length;
}

void GDFlushCurrToMem() {
    DCFlushRange(__GDCurrentDL->start, __GDCurrentDL->length);
}

void GDPadCurr32() {
#ifdef GALAXY_NATIVE
    u32 n = static_cast<u32>(reinterpret_cast<uintptr_t>(__GDCurrentDL->ptr) & 31);
#else
    u32 n = ((u32)__GDCurrentDL->ptr & 31);
#endif
    if (n) {
        for (; n < 32; n++) {
            __GDWrite(0);
        }
    }
}

void GDOverflowed() {
    if (overflowcb) {
        (*overflowcb)();
    }
}
