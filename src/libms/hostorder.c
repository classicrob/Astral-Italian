// Native only: LibMessageStudio binaries are big-endian on the disc, and libms plus its game-side
// consumers read their fields with plain loads. Before the first analysis each binary is rewritten
// in place to host byte order, once (the byte-order mark records it), so the original readers work:
//   header/block headers, LBL1/FEN1 label tables, ATR1 entries, TXT2 offsets and UTF-16 text
//   (host-order code units, as the native text handling expects), FLW2 nodes and branches.
#ifdef GALAXY_NATIVE
#include "libms.h"

#define LMS_BOM_HOST 0xFEFF

static u16 swap16(u16 v) {
    return (u16)((v >> 8) | (v << 8));
}

// Unaligned in-place swaps (label entries put their u32 index at odd offsets).
static void fix16(void* p) {
    u8* b = (u8*)p;
    u8 t = b[0];
    b[0] = b[1];
    b[1] = t;
}

static void fix32(void* p) {
    u8* b = (u8*)p;
    u8 t = b[0];
    b[0] = b[3];
    b[3] = t;
    t = b[1];
    b[1] = b[2];
    b[2] = t;
}

static u32 be32(const void* p) {
    const u8* b = (const u8*)p;
    return ((u32)b[0] << 24) | ((u32)b[1] << 16) | ((u32)b[2] << 8) | b[3];
}

static BOOL isBlock(const u8* pBlock, const char* pName) {
    return LMSi_MemCmp(pBlock, pName, 4);
}

// LBL1/FEN1: u32 bucketNum, {u32 count, u32 offset}[bucketNum], then {u8 len, char[len], u32 index} entries.
static void fixLabels(u8* pData) {
    u32 bucketNum = be32(pData);
    u32 i;
    u32 j;

    for (i = 0; i < bucketNum; i++) {
        u8* pBucket = pData + 4 + i * 8;
        u32 count = be32(pBucket);
        u8* pEntry = pData + be32(pBucket + 4);
        for (j = 0; j < count; j++) {
            fix32(pEntry + 1 + pEntry[0]);
            pEntry += pEntry[0] + 5;
        }
        fix32(pBucket);
        fix32(pBucket + 4);
    }
    fix32(pData);
}

// ATR1: u32 count, u32 entrySize, entries {u8 x4, u16, u8 x2[, u32 string offset]}.
static void fixAttributes(u8* pData) {
    u32 count = be32(pData);
    u32 size = be32(pData + 4);
    u32 i;

    for (i = 0; i < count; i++) {
        u8* pEntry = pData + 8 + i * size;
        if (size >= 8) {
            fix16(pEntry + 4);
        }
        if (size >= 12) {
            fix32(pEntry + 8);
        }
    }
    fix32(pData);
    fix32(pData + 4);
}

// TXT2: u32 count, u32 offset[count], then UTF-16 strings up to the end of the block.
static void fixText(u8* pData, u32 blockSize) {
    u32 count = be32(pData);
    u32 i;

    if (count != 0) {
        u8* p;
        for (p = pData + be32(pData + 4); p + 1 < pData + blockSize; p += 2) {
            fix16(p);
        }
    }
    for (i = 0; i <= count; i++) {
        fix32(pData + i * 4);
    }
}

// FLW2: u16 nodeNum, u16 branchNum, 4 bytes, 12-byte nodes of u16 fields (an event node, type 3,
// keeps a u32 parameter at +8), then u16 branch indices.
static void fixFlow(u8* pData) {
    u16 nodeNum = (u16)((pData[0] << 8) | pData[1]);
    u16 branchNum = (u16)((pData[2] << 8) | pData[3]);
    u32 i;

    for (i = 0; i < nodeNum; i++) {
        u8* pNode = pData + 8 + i * 12;
        BOOL event = pNode[0] == 0 && pNode[1] == 3;
        fix16(pNode);
        fix16(pNode + 2);
        fix16(pNode + 4);
        fix16(pNode + 6);
        if (event) {
            fix32(pNode + 8);
        } else {
            fix16(pNode + 8);
            fix16(pNode + 10);
        }
    }
    for (i = 0; i < branchNum; i++) {
        fix16(pData + 8 + nodeNum * 12 + i * 2);
    }
    fix16(pData);
    fix16(pData + 2);
}

void LMSi_ConvertToHostOrder(MsbHeader* pHeader) {
    u8* pFile = (u8*)pHeader;
    u16 blockCount;
    u32 offset = sizeof(MsbHeader);
    u16 i;

    if (pHeader->bom == LMS_BOM_HOST) {
        return;
    }

    blockCount = swap16(pHeader->blockCount);
    for (i = 0; i < blockCount; i++) {
        u8* pBlock = pFile + offset;
        u32 size = be32(pBlock + 4);
        u8* pData = pBlock + sizeof(MsbBlockHeader);

        if (isBlock(pBlock, "LBL1") || isBlock(pBlock, "FEN1")) {
            fixLabels(pData);
        } else if (isBlock(pBlock, "ATR1")) {
            fixAttributes(pData);
        } else if (isBlock(pBlock, "TXT2")) {
            fixText(pData, size);
        } else if (isBlock(pBlock, "FLW2")) {
            fixFlow(pData);
        }
        fix32(pBlock + 4);
        fix16(pBlock + 8);
        offset += sizeof(MsbBlockHeader) + size;
        offset = (offset + 0xF) & ~0xF;
    }

    pHeader->bom = LMS_BOM_HOST;
    pHeader->_A = swap16(pHeader->_A);
    pHeader->blockCount = blockCount;
    pHeader->_10 = swap16(pHeader->_10);
    fix32(&pHeader->fileSize);
}
#endif
