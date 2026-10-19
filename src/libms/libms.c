#include "libms.h"

static LMSAllocFunc sAllocFunc;
static LMSFreeFunc sFreeFunc;

void LMS_SetMemFuncs(LMSAllocFunc pAlloc, LMSFreeFunc pFree) {
    sAllocFunc = pAlloc;
    sFreeFunc = pFree;
}

void* LMSi_Malloc(u32 size) {
    return sAllocFunc(size);
}

void LMSi_Free(void* pPtr) {
    sFreeFunc(pPtr);
}

BOOL LMSi_MemCmp(const void* pA, const void* pB, s32 size) {
    s32 i;

    for (i = 0; i < size; i++) {
        if (((const char*)pA)[i] != ((const char*)pB)[i]) {
            return FALSE;
        }
    }

    return TRUE;
}
