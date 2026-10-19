#include "libms.h"

MsbtInfo* LMS_InitMessage(void* pData) {
    MsbtInfo* pInfo = (MsbtInfo*)LMSi_Malloc(sizeof(MsbtInfo));

    pInfo->base.header = (MsbHeader*)pData;
    LMSi_AnalyzeMessageBinary(&pInfo->base, "MsgStdBn");
    pInfo->lbl1Index = LMSi_SearchBlockByName(&pInfo->base, "LBL1");
    pInfo->txt2Index = LMSi_SearchBlockByName(&pInfo->base, "TXT2");
    pInfo->atr1Index = LMSi_SearchBlockByName(&pInfo->base, "ATR1");
    return pInfo;
}

// NONMATCHING: register allocation only (bucket*2 temporary colored r3 instead of r4/r5)
s32 LMS_GetTextIndexByLabel(MsbtInfo* pInfo, const char* pLabel) {
    const MsbBlock* pBlocks;
    s32 bucket;
    u32 i;
    u32 entryNum;
    s32 offset;
    s32 entryLen;
    s32 blockIndex;
    s32 labelLen;

    blockIndex = pInfo->lbl1Index;
    labelLen = 0;
    if (pInfo->lbl1Index == -1) {
        return LMS_MISSING_LBL1_DATA;
    } else {
        while (pLabel[labelLen] != '\0') {
            labelLen++;
        }
        pBlocks = pInfo->base.blocks;
        bucket = LMSi_GetHashTableIndexFromLabel(pLabel, pBlocks[blockIndex].data[0]);
        bucket *= 2;
        entryNum = pBlocks[blockIndex].data[++bucket];
        offset = pBlocks[blockIndex].data[++bucket];
        for (i = 0; i < entryNum; i++) {
            entryLen = *(u8*)((char*)pBlocks[blockIndex].data + offset);
            if (entryLen == labelLen && LMSi_MemCmp(pLabel, (char*)pBlocks[blockIndex].data + (offset + 1), entryLen)) {
                return *(s32*)((char*)pBlocks[blockIndex].data + (offset + entryLen + 1));
            }
            offset += entryLen + 5;
        }
    }

    return LMS_NOT_FOUND;
}

const wchar_t* LMS_GetText(MsbtInfo* pInfo, s32 index) {
    s32* pTable;

    if (pInfo->txt2Index == -1) {
        return NULL;
    }

    pTable = (s32*)pInfo->base.blocks[pInfo->txt2Index].data;
    if (pTable[0] <= index) {
        return NULL;
    }

    return (const wchar_t*)((char*)pTable + pTable[++index]);
}

const wchar_t* LMS_GetTextByLabel(MsbtInfo* pInfo, const char* pLabel) {
    s32 index = LMS_GetTextIndexByLabel(pInfo, pLabel);

    if (index < 0) {
        return NULL;
    } else {
        return LMS_GetText(pInfo, index);
    }
}

MsbtAttribute* LMS_GetAttribute(MsbtInfo* pInfo, s32 index) {
    s32* pTable = (s32*)pInfo->base.blocks[pInfo->atr1Index].data;

    return (MsbtAttribute*)((char*)pTable + (pTable[1] * index + 8));
}
