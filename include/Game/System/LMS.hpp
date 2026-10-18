#pragma once

#include <revolution/types.h>

// Libs of the Nintendo message system (LMS). Plain C symbols in the target.
struct LMSAttribute {
    /* 0x0 */ u8 _0;
    /* 0x1 */ u8 _1;
    /* 0x2 */ u8 _2;
    /* 0x3 */ u8 _3;
    /* 0x4 */ u16 _4;
    /* 0x6 */ u8 _6;
    /* 0x7 */ u8 _7;
};

extern "C" {
void* LMS_InitMessage(void* pData);
void* LMS_InitFlowchart(void* pData);
u8* LMS_GetTextByLabel(void* pMsg, const char* pLabel);
s32 LMS_GetTextIndexByLabel(void* pMsg, const char* pLabel);
u8* LMS_GetText(void* pMsg, s32 index);
LMSAttribute* LMS_GetAttribute(void* pMsg, s32 index);
s32 LMS_GetEntryNodeIndex(void* pFlow, const char* pLabel);
void* LMS_GetNodeBranchPtr(void* pFlow, u32 nodeIndex);
void* LMS_GetNodeDataPtr(void* pFlow, u32 nodeIndex);
s32 LMS_GetFlowNodeIndex(void* pFlow);
void LMS_SetMemFuncs(void* (*pAlloc)(u32), void (*pFree)(void*), void* (*pAlloc2)(u32));
}
