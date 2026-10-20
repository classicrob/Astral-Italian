#ifndef LIBMS_H
#define LIBMS_H

#include <revolution/types.h>
#ifdef GALAXY_NATIVE
#include <petari/utf16.h>
#else
#include <wchar_t.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// LibMessageStudio (LMS): reader for the "MsgStdBn" (.msbt) and "MsgFlwBn" (.msbf) binaries.

typedef void* (*LMSAllocFunc)(u32 size);
typedef void (*LMSFreeFunc)(void* pPtr);

// Common 0x20-byte file header of both binaries; the first block follows it.
#pragma pack(push, 2)
typedef struct MsbHeader {
    /* 0x00 */ char magic[8];
    /* 0x08 */ u16 bom;
    /* 0x0A */ u16 _A;
    /* 0x0C */ u8 version;
    /* 0x0D */ u8 _D;
    /* 0x0E */ u16 blockCount;
    /* 0x10 */ u16 _10;
    /* 0x12 */ s32 fileSize;
    /* 0x16 */ u8 _16[0xA];
} MsbHeader;
#pragma pack(pop)

// Header in front of every block, 0x10-aligned in the file.
typedef struct MsbBlockHeader {
    /* 0x00 */ char name[4];
    /* 0x04 */ s32 size;
    /* 0x08 */ u16 _8;
    /* 0x0A */ u8 _A[6];
} MsbBlockHeader;

typedef struct MsbBlock {
    /* 0x00 */ s32* data;
    /* 0x04 */ char name[4];
    /* 0x08 */ s32 size;
    /* 0x0C */ u16 _C;
} MsbBlock;

typedef struct MsbInfo {
    /* 0x00 */ MsbHeader* header;
    /* 0x04 */ s32 fileSize;
    /* 0x08 */ u8 version;
    /* 0x0A */ u16 blockCount;
    /* 0x0C */ MsbBlock* blocks;
} MsbInfo;

// Label table (LBL1 / FEN1): u32 bucketNum, then {u32 count, u32 offset} per bucket;
// each entry is u8 length, label chars, u32 value.
typedef struct MsbLabelBucket {
    /* 0x00 */ u32 count;
    /* 0x04 */ u32 offset;
} MsbLabelBucket;

// Message file (.msbt) handle.
typedef struct MsbtInfo {
    /* 0x00 */ MsbInfo base;
    /* 0x10 */ s32 lbl1Index;
    /* 0x14 */ s32 txt2Index;
    /* 0x18 */ s32 atr1Index;
} MsbtInfo;

// Flow file (.msbf) handle.
typedef struct MsbfInfo {
    /* 0x00 */ MsbInfo base;
    /* 0x10 */ s32 flw2Index;
    /* 0x14 */ s32 fen1Index;
    /* 0x18 */ s32 ref1Index;
} MsbfInfo;

// FLW2 block: u16 nodeNum, u16 branchNum, 4 bytes pad, nodeNum nodes, then the u16 branch table.
typedef struct MsbfNode {
    /* 0x00 */ u16 type;
    /* 0x02 */ u8 _2[8];
    /* 0x0A */ u16 branchIndex;
} MsbfNode;

typedef struct MsbtAttribute {
    /* 0x0 */ u8 _0;
    /* 0x1 */ u8 _1;
    /* 0x2 */ u8 _2;
    /* 0x3 */ u8 _3;
    /* 0x4 */ u16 _4;
    /* 0x6 */ u8 _6;
    /* 0x7 */ u8 _7;
} MsbtAttribute;

enum {
    LMS_NOT_FOUND = -1,
    LMS_MISSING_LBL1_DATA = -2,
    LMS_MISSING_FLW2_DATA = -3,
    LMS_MISSING_FEN1_DATA = -9,
};

// libms.c
void LMS_SetMemFuncs(LMSAllocFunc pAlloc, LMSFreeFunc pFree);
void* LMSi_Malloc(u32 size);
void LMSi_Free(void* pPtr);
BOOL LMSi_MemCmp(const void* pA, const void* pB, s32 size);

// commonlib.c
void LMSi_AnalyzeMessageHeader(MsbInfo* pInfo);
void LMSi_AnalyzeMessageBlocks(MsbInfo* pInfo);
void LMSi_AnalyzeMessageBinary(MsbInfo* pInfo, const char* pType);
s32 LMSi_SearchBlockByName(MsbInfo* pInfo, const char* pName);
s32 LMSi_GetHashTableIndexFromLabel(const char* pLabel, s32 bucketNum);

#ifdef GALAXY_NATIVE
// hostorder.c: rewrites a big-endian binary to host byte order in place, once.
void LMSi_ConvertToHostOrder(MsbHeader* pHeader);
#endif

// flowfile.c
MsbfInfo* LMS_InitFlowchart(void* pData);
s32 LMS_GetNodeNum(MsbfInfo* pInfo);
s32 LMS_GetEntryNodeIndex(MsbfInfo* pInfo, const char* pLabel);
MsbfNode* LMS_GetNodeDataPtr(MsbfInfo* pInfo, s32 index);
u32 LMS_GetFlowNodeIndex(MsbfInfo* pInfo, const MsbfNode* pNode);
u16* LMS_GetNodeBranchPtr(MsbfInfo* pInfo, s32 index);

// msgfile.c
MsbtInfo* LMS_InitMessage(void* pData);
s32 LMS_GetTextIndexByLabel(MsbtInfo* pInfo, const char* pLabel);
const wchar_t* LMS_GetText(MsbtInfo* pInfo, s32 index);
const wchar_t* LMS_GetTextByLabel(MsbtInfo* pInfo, const char* pLabel);
MsbtAttribute* LMS_GetAttribute(MsbtInfo* pInfo, s32 index);

#ifdef __cplusplus
}
#endif

#endif
