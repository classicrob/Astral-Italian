#include "libms.h"

MsbfInfo* LMS_InitFlowchart(void* pData) {
    MsbfInfo* pInfo = (MsbfInfo*)LMSi_Malloc(sizeof(MsbfInfo));

    pInfo->base.header = (MsbHeader*)pData;
    LMSi_AnalyzeMessageBinary(&pInfo->base, "MsgFlwBn");
    pInfo->flw2Index = LMSi_SearchBlockByName(&pInfo->base, "FLW2");
    pInfo->fen1Index = LMSi_SearchBlockByName(&pInfo->base, "FEN1");
    pInfo->ref1Index = LMSi_SearchBlockByName(&pInfo->base, "REF1");
    return pInfo;
}

s32 LMS_GetNodeNum(MsbfInfo* pInfo) {
    if (pInfo->flw2Index == -1) {
        return LMS_MISSING_FLW2_DATA;
    }

    return *(u16*)pInfo->base.blocks[pInfo->flw2Index].data;
}

// NONMATCHING: register allocation only (bucket*2 temporary colored r3 instead of r4/r5)
s32 LMS_GetEntryNodeIndex(MsbfInfo* pInfo, const char* pLabel) {
    const MsbBlock* pBlocks;
    s32 bucket;
    u32 i;
    u32 entryNum;
    s32 offset;
    s32 entryLen;
    s32 blockIndex;
    s32 labelLen;

    blockIndex = pInfo->fen1Index;
    labelLen = 0;
    if (pInfo->fen1Index == -1) {
        return LMS_MISSING_FEN1_DATA;
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

MsbfNode* LMS_GetNodeDataPtr(MsbfInfo* pInfo, s32 index) {
    if (pInfo->flw2Index == -1) {
        return NULL;
    }

    return (MsbfNode*)((char*)pInfo->base.blocks[pInfo->flw2Index].data + 8) + index;
}

u32 LMS_GetFlowNodeIndex(MsbfInfo* pInfo, const MsbfNode* pNode) {
    return (u32)((const char*)pNode - (const char*)pInfo->base.blocks[pInfo->flw2Index].data - 8) / sizeof(MsbfNode);
}

u16* LMS_GetNodeBranchPtr(MsbfInfo* pInfo, s32 index) {
    s32 nodeNum = LMS_GetNodeNum(pInfo);
    const MsbBlock* pBlock = &pInfo->base.blocks[pInfo->flw2Index];
    MsbfNode* pNode = LMS_GetNodeDataPtr(pInfo, index);

    if (pNode == NULL) {
        return NULL;
    }

    if (pNode->type != 2) {
        return NULL;
    }

    return (u16*)((MsbfNode*)((char*)pBlock->data + 8) + nodeNum) + pNode->branchIndex;
}
