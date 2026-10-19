#pragma once

#include <revolution/types.h>

class HashSortTable;
class ResourceHolder;

class XanimeSwapTable {
public:
    /* 0x0 */ const char* mOriginal;
    /* 0x4 */ const char* mSwapped;
};

class XanimeBckTable {
public:
    /* 0x0 */ const char* mAnimationName;
};

class XanimeOfsInfo {
public:
    /* 0x00 */ XanimeBckTable mParent;
    /* 0x04 */ f32 mStartFrame;
    /* 0x08 */ f32 mEndFrame;
    /* 0x0C */ f32 mLoopFrame;
    /* 0x10 */ u32 mAttribute;
};

class XanimeGroupInfo {
public:
    /* 0x00 */ XanimeBckTable mParent;
    /* 0x04 */ f32 mRate;
    /* 0x08 */ u32 mInterpoleFrame;
    /* 0x0C */ f32 mStart;
    /* 0x10 */ f32 mEnd;
    /* 0x14 */ f32 mLoop;
   /* 0x18 */ s32 mAttribute;
    /* 0x1C */ u8 mBckTableVariant;
    /* 0x1D */ u8 mReserved;
    /* 0x20 */ void* mAnimations[4];
    /* 0x30 */ f32 mWeights[4];
    /* 0x40 */ u32 mAnimationHash;
    /* 0x44 */ const char* mBckName;

    void init();
};

class XanimeDirectory {
public:
    enum DIRECTORY_TYPE { Recursive = 0, Leaf = 1 };

    /* 0x00 */ const char* mName;
    /* 0x04 */ u8 mDirectoryType;
    /* 0x08 */ void* mChildren;
    /* 0x0C */ u32 mHash;
    /* 0x10 */ u32 mSize;
};

struct XanimeBckTableEntry {
    /* 0x00 */ const char* mFileName;
    /* 0x04 */ f32 mWeight;
};

class XanimeBckTable1 {
public:
    /* 0x0 */ XanimeBckTable mParent;
    /* 0x4 */ const char* mFileName;
    /* 0x40 */ u32 mAnimationHash;
    /* 0xC */ u32 mFileHash;
};

class XanimeBckTable2 {
public:
    /* 0x0 */ XanimeBckTable mParent;
    /* 0x4 */ XanimeBckTableEntry mEntries[2];
};

class XanimeBckTable3 {
public:
    /* 0x0 */ XanimeBckTable mParent;
    /* 0x4 */ XanimeBckTableEntry mEntries[3];
};

class XanimeBckTable4 {
public:
    /* 0x0 */ XanimeBckTable mParent;
    /* 0x4 */ XanimeBckTableEntry mEntries[4];
};

// size is 0x18
class XanimeAuxInfo {
public:
    /* 0x0 */ XanimeBckTable mParent;
    /* 0x04 */ u8 mUnusedFlag;
    /* 0x08 */ XanimeOfsInfo* mUnusedOfsInfo;

    // Unreferenced members
    /* 0x1D */ u8 mReserved[12];
};

class XanimeResourceTable {
public:
    XanimeResourceTable(ResourceHolder*, XanimeGroupInfo*, XanimeOfsInfo*, XanimeBckTable1*, XanimeBckTable2*, XanimeBckTable3*, XanimeBckTable4*, XanimeSwapTable*);
    XanimeResourceTable(ResourceHolder*);
    u32 initGroupInfo(ResourceHolder*, XanimeGroupInfo*, XanimeOfsInfo*, XanimeBckTable1*, XanimeBckTable2*, XanimeBckTable3*, XanimeBckTable4*, XanimeSwapTable*);

    const XanimeGroupInfo* getGroupInfo(const char*) const;
    const XanimeGroupInfo* getGroupInfo(const char*, XanimeDirectory*) const;
    XanimeGroupInfo* getGroupInfoFromHash(u32) const;
    u32 getIndex(XanimeDirectory*, const char*) const;
    u32 getIndexFromHash(u32) const;
    u32 getGroupIndex(const char*) const;
    u32 getSingleIndex(const char*) const;
    u32 getSimpleIndex(const char*) const;
    const char* swapBckName(const char*, XanimeSwapTable*) const;
    void* findResMotion(const char*) const;
    const char* findStringMotion(const char*) const;
    void createSortTable();
    bool search(XanimeBckTable**, const char*, u32) const;
    void init();

    /* 0x00 */ u8 mHasGroupTable;
    /* 0x01 */ u8 mMaxGroupInfoTableSize;
    /* 0x04 */ u32 mAmountOfGroupInfos;
    /* 0x08 */ u32 mAmountOfBckTables;
    /* 0x0C */ u32 mAmountOfSimpleGroupInfos;
    /* 0x10 */ XanimeGroupInfo* mGroupInfos;
    /* 0x14 */ XanimeBckTable1* mBckTables;
    /* 0x18 */ XanimeGroupInfo mDefaultGroup;
    /* 0x60 */ XanimeGroupInfo* mSimpleGroupInfos;
    /* 0x64 */ ResourceHolder* mResourceHolder;
    /* 0x68 */ HashSortTable* mSortTable;
    /* 0x6C */ XanimeSwapTable* mSwapTable;
};
