#include "libms.h"

void LMSi_AnalyzeMessageHeader(MsbInfo* pInfo) {
    pInfo->version = pInfo->header->version;
    pInfo->blockCount = pInfo->header->blockCount;
    if (pInfo->blockCount == 0) {
        pInfo->blocks = NULL;
    } else {
        pInfo->blocks = (MsbBlock*)LMSi_Malloc(pInfo->blockCount * sizeof(MsbBlock));
    }
    pInfo->fileSize = pInfo->header->fileSize;
}

// NONMATCHING: register allocation only (offset/pos/block colored r11/r9/r10 instead of r9/r10/r11)
void LMSi_AnalyzeMessageBlocks(MsbInfo* pInfo) {
    s32 i;
    s32 j;
    s32 pos;
    MsbBlock* pBlock;
    s32 offset = sizeof(MsbHeader);

    for (i = 0; i < pInfo->blockCount; i++) {
        pBlock = &pInfo->blocks[i];
        pos = offset;
        pBlock->data = (s32*)((char*)pInfo->header + offset + sizeof(MsbBlockHeader));
        for (j = 0; j < 4; j++) {
            pBlock->name[j] = *((char*)pInfo->header + pos);
            pos++;
        }
        pBlock->size = *(s32*)((char*)pInfo->header + pos);
        offset += sizeof(MsbBlockHeader);
        offset += pBlock->size;
        offset = (offset + 0xF) & ~0xF;
        pBlock->_C = *(u16*)((char*)pInfo->header + pos + 4);
    }
}

void LMSi_AnalyzeMessageBinary(MsbInfo* pInfo, const char* pType) {
    LMSi_AnalyzeMessageHeader(pInfo);
    LMSi_AnalyzeMessageBlocks(pInfo);
}

s32 LMSi_SearchBlockByName(MsbInfo* pInfo, const char* pName) {
    u16 i;

    for (i = 0; i < pInfo->blockCount; i++) {
        if (LMSi_MemCmp(pInfo->blocks[i].name, pName, 4)) {
            return i;
        }
    }

    return -1;
}

s32 LMSi_GetHashTableIndexFromLabel(const char* pLabel, s32 bucketNum) {
    u32 hash = 0;
    s32 c;

    while (TRUE) {
        c = *pLabel;
        if (c == '\0') {
            break;
        }
        hash = hash * 0x492 + c;
        pLabel++;
    }

    return hash % bucketNum;
}
