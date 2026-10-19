#include "Game/Animation/XanimeResource.hpp"
#ifdef GALAXY_NATIVE
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#endif
#include "Game/System/ResourceHolder.hpp"
#include "Game/System/ResourceInfo.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <cstring>
#include <revolution/types.h>

void XanimeBckTable3::init() {
    mParent.mAnimationName = nullptr;
    mEntries[0].mFileName = nullptr;
    mEntries[0].mWeight = 1.0f;
    mEntries[1].mFileName = nullptr;
    mEntries[1].mWeight = 0.0f;
    mEntries[2].mFileName = nullptr;
    mEntries[2].mWeight = 0.0f;
}

void XanimeGroupInfo::init() {
    mParent.mAnimationName = "not-initialized";
    mRate = 0.0f;
    mInterpoleFrame = 0;
    mStart = 0.0f;
    mEnd = 0.0f;
    mLoop = 0.0f;
    mAttribute = 0;
    mBckTableVariant = 0;
    for (u32 i = 0; i < 4; i++) {
        mWeights[i] = 0.0f;
        mAnimations[i] = nullptr;
    }
    mAnimationHash = 0;
}

static bool unknownByte;

XanimeResourceTable::XanimeResourceTable(ResourceHolder* pArg) {
    init();

    mHasGroupTable = 0;
    mGroupInfos = nullptr;
    mResourceHolder = pArg;
    mAmountOfBckTables = 0;
    mAmountOfGroupInfos = 0;

    mMaxGroupInfoTableSize = 1;

    if (unknownByte != 0) {
        mMaxGroupInfoTableSize = unknownByte;
    }

    mDefaultGroup.init();

    mDefaultGroup.mRate = 1.0f;
    mDefaultGroup.mParent.mAnimationName = "non-group";
    mDefaultGroup.mInterpoleFrame = 1;
    mDefaultGroup.mBckTableVariant = 1;
}

XanimeResourceTable::XanimeResourceTable(ResourceHolder* pResourceHolder, XanimeGroupInfo* pGroupInfo,
                                         XanimeOfsInfo* pOfsInfo, XanimeBckTable1* pBckTable1, XanimeBckTable2* pBckTable2,
                                         XanimeBckTable3* pBckTable3, XanimeBckTable4* pBckTable4, XanimeSwapTable* pSwapTable) {
    init();
    mHasGroupTable = 1;
    mGroupInfos = pGroupInfo;
    mResourceHolder = pResourceHolder;
    mMaxGroupInfoTableSize = 1;
    mAmountOfGroupInfos = initGroupInfo(pResourceHolder, pGroupInfo, pOfsInfo, pBckTable1, pBckTable2, pBckTable3, pBckTable4, pSwapTable);
    createSortTable();

    mBckTables = reinterpret_cast< XanimeBckTable1* >(pBckTable1);

    int i = 0;
    while (mBckTables != nullptr) {
        if (mBckTables[i].mParent.mAnimationName[0] == '\0') {
            break;
        }

        mBckTables[i].mAnimationHash = MR::getHashCode(mBckTables[i].mParent.mAnimationName);
        mBckTables[i].mFileHash = MR::getHashCode(mBckTables[i].mFileName);

        i++;
    }

    mAmountOfBckTables = i;

    mDefaultGroup.init();
    mDefaultGroup.mRate = 1.0f;
    mDefaultGroup.mParent.mAnimationName = "non-group";
    mDefaultGroup.mInterpoleFrame = 1;
    mDefaultGroup.mBckTableVariant = 1;
    mSimpleGroupInfos = new XanimeGroupInfo[0];
    mAmountOfSimpleGroupInfos = 0;
}

void XanimeResourceTable::init() {
    mHasGroupTable = 0;
    mMaxGroupInfoTableSize = 0;
    mAmountOfGroupInfos = 0;
    mAmountOfBckTables = 0;
    mAmountOfSimpleGroupInfos = 0;
    mGroupInfos = nullptr;
    mBckTables = nullptr;

    mResourceHolder = nullptr;
    mSortTable = nullptr;
    mSwapTable = nullptr;
}

// NONMATCHING: local table arrays occupy different stack slots.
u32 XanimeResourceTable::initGroupInfo(ResourceHolder* pResourceHolder, XanimeGroupInfo* pInfo,  XanimeOfsInfo* pOfsInfo,
                                       XanimeBckTable1* pBckTable1, XanimeBckTable2* pBckTable2, XanimeBckTable3* pBckTable3,
                                       XanimeBckTable4* pBckTable4, XanimeSwapTable* pSwapTable) {
    mSwapTable = pSwapTable;
    int i = 0;
    u8 maxTableSize = 1;
    while (true) {
        XanimeGroupInfo* entry = &pInfo[i];
        if (entry->mParent.mAnimationName[0] == '\0') {
            break;
        }

        for (int j = 0; j < ARRAY_SIZE(entry->mAnimations); j++) {
            entry->mAnimations[j] = nullptr;
            entry->mWeights[j] = 0.0f;
        }

        entry->mAnimationHash = MR::getHashCode(entry->mParent.mAnimationName);

        XanimeBckTable* bckTables[4];
        bckTables[3] = reinterpret_cast< XanimeBckTable* >(pBckTable1);
        bckTables[2] = reinterpret_cast< XanimeBckTable* >(pBckTable2);
        bckTables[1] = reinterpret_cast< XanimeBckTable* >(pBckTable3);
        bckTables[0] = reinterpret_cast< XanimeBckTable* >(pBckTable4);

        entry->mBckTableVariant = 0;

        const char* firstFilename = nullptr;
        if (search(&bckTables[3], entry->mParent.mAnimationName, sizeof(XanimeBckTable1))) {
            entry->mBckTableVariant = 1;

            entry->mWeights[0] = 1.0f;

            pResourceHolder->mMotionResTable->findFileInfo(reinterpret_cast< XanimeBckTable1* >(bckTables[3])->mFileName);

            entry->mAnimations[0] = findResMotion(reinterpret_cast< XanimeBckTable1* >(bckTables[3])->mFileName);

            firstFilename = reinterpret_cast< XanimeBckTable1* >(bckTables[3])->mFileName;
        } else if (search(&bckTables[2], entry->mParent.mAnimationName, sizeof(XanimeBckTable2))) {
            if (maxTableSize < 2) {
                maxTableSize = 2;
            }

            entry->mBckTableVariant = 2;

            entry->mAnimations[0] = findResMotion(reinterpret_cast< XanimeBckTable2* >(bckTables[2])->mEntries[0].mFileName);
            entry->mWeights[0] = reinterpret_cast< XanimeBckTable2* >(bckTables[2])->mEntries[0].mWeight;

            entry->mAnimations[1] = findResMotion(reinterpret_cast< XanimeBckTable2* >(bckTables[2])->mEntries[1].mFileName);
            entry->mWeights[1] = reinterpret_cast< XanimeBckTable2* >(bckTables[2])->mEntries[1].mWeight;

            firstFilename = reinterpret_cast< XanimeBckTable2* >(bckTables[2])->mEntries[0].mFileName;
        } else if (search(&bckTables[1], entry->mParent.mAnimationName, sizeof(XanimeBckTable3))) {
            if (maxTableSize < 3) {
                maxTableSize = 3;
            }

            entry->mBckTableVariant = 3;

            entry->mAnimations[0] = findResMotion(reinterpret_cast< XanimeBckTable3* >(bckTables[1])->mEntries[0].mFileName);
            entry->mWeights[0] = reinterpret_cast< XanimeBckTable3* >(bckTables[1])->mEntries[0].mWeight;

            entry->mAnimations[1] = findResMotion(reinterpret_cast< XanimeBckTable3* >(bckTables[1])->mEntries[1].mFileName);
            entry->mWeights[1] = reinterpret_cast< XanimeBckTable3* >(bckTables[1])->mEntries[1].mWeight;

            entry->mAnimations[2] = findResMotion(reinterpret_cast< XanimeBckTable3* >(bckTables[1])->mEntries[2].mFileName);
            entry->mWeights[2] = reinterpret_cast< XanimeBckTable3* >(bckTables[1])->mEntries[2].mWeight;

            firstFilename = reinterpret_cast< XanimeBckTable3* >(bckTables[1])->mEntries[0].mFileName;
        } else if (search(&bckTables[0], entry->mParent.mAnimationName, sizeof(XanimeBckTable4))) {
            if (maxTableSize < 4) {
                maxTableSize = 4;
            }

            entry->mBckTableVariant = 4;

            entry->mAnimations[0] = findResMotion(reinterpret_cast< XanimeBckTable4* >(bckTables[0])->mEntries[0].mFileName);
            entry->mWeights[0] = reinterpret_cast< XanimeBckTable4* >(bckTables[0])->mEntries[0].mWeight;

            entry->mAnimations[1] = findResMotion(reinterpret_cast< XanimeBckTable4* >(bckTables[0])->mEntries[1].mFileName);
            entry->mWeights[1] = reinterpret_cast< XanimeBckTable4* >(bckTables[0])->mEntries[1].mWeight;

            entry->mAnimations[2] = findResMotion(reinterpret_cast< XanimeBckTable4* >(bckTables[0])->mEntries[2].mFileName);
            entry->mWeights[2] = reinterpret_cast< XanimeBckTable4* >(bckTables[0])->mEntries[2].mWeight;

            entry->mAnimations[3] = findResMotion(reinterpret_cast< XanimeBckTable4* >(bckTables[0])->mEntries[3].mFileName);
            entry->mWeights[3] = reinterpret_cast< XanimeBckTable4* >(bckTables[0])->mEntries[3].mWeight;

            firstFilename = reinterpret_cast< XanimeBckTable4* >(bckTables[0])->mEntries[0].mFileName;
        }

        entry->mBckName = firstFilename;

        entry->mStart = 0.0f;
        if (entry->mAnimations[0] == nullptr) {
            entry->mEnd = 0.0f;
            entry->mLoop = 0.0f;
            entry->mAttribute = 0;
        } else {
#ifdef GALAXY_NATIVE
            const J3DAnmBase* animation = static_cast<const J3DAnmBase*>(entry->mAnimations[0]);
            entry->mAttribute = animation->getAttribute();
            entry->mLoop = 0.0f;
            entry->mEnd = static_cast<f32>(animation->getFrameMax());
#else
            entry->mAttribute = static_cast< u8* >(entry->mAnimations[0])[4];
            entry->mLoop = 0.0f;
            entry->mEnd = static_cast< f32 >(reinterpret_cast< const s16* >(entry->mAnimations[0])[3]);
#endif
        }

        XanimeBckTable* ofsTables[1];
        ofsTables[0] = reinterpret_cast< XanimeBckTable* >(pOfsInfo);

        if (search(ofsTables, entry->mParent.mAnimationName, sizeof(XanimeOfsInfo))) {
            if (reinterpret_cast<XanimeOfsInfo*>(ofsTables[0])->mStartFrame >= 0.0f)
                entry->mStart = reinterpret_cast< XanimeOfsInfo* >(ofsTables[0])->mStartFrame;
            if (reinterpret_cast<XanimeOfsInfo*>(ofsTables[0])->mLoopFrame >= 0.0f)
                entry->mLoop = reinterpret_cast< XanimeOfsInfo* >(ofsTables[0])->mLoopFrame;
            if (reinterpret_cast<XanimeOfsInfo*>(ofsTables[0])->mEndFrame >= 0.0f)
                entry->mEnd = reinterpret_cast< XanimeOfsInfo* >(ofsTables[0])->mEndFrame;
            entry->mAttribute = reinterpret_cast< XanimeOfsInfo* >(ofsTables[0])->mAttribute;
        }

        i++;
    }

    mMaxGroupInfoTableSize = maxTableSize;

    return i;
}

const XanimeGroupInfo* XanimeResourceTable::getGroupInfo(const char* pArg) const {
    switch (mHasGroupTable) {
    case 0:
        return nullptr;

    case 1:
        s32 groupIndex = getGroupIndex(pArg);
        if (groupIndex == -1) {
            s32 simpleIndex = getSimpleIndex(pArg);
            if (simpleIndex == -1) {
                return nullptr;
            }

            return &mSimpleGroupInfos[simpleIndex];
        }

        return &mGroupInfos[groupIndex];

    }

    return nullptr;
}

const XanimeGroupInfo* XanimeResourceTable::getGroupInfo(const char* pPath, XanimeDirectory* pDir) const {
    int i = 0;
    while (true) {
        i++;

        if (pPath[i] == '\0') {
            break;
        }

        if (pPath[i] == '/') {
            char pathOneDirectoryDown[32];
            MR::extractString(pathOneDirectoryDown, &pPath[i], i, 32);

            u32 index = getIndex(pDir, pathOneDirectoryDown);

            switch (pDir[index].mDirectoryType) {
            case XanimeDirectory::Recursive:
                return getGroupInfo(&pPath[i + 1], static_cast<XanimeDirectory*>(pDir[index].mChildren));

            case XanimeDirectory::Leaf:
                u32 hash = MR::getHashCode(&pPath[i + 1]);
                XanimeGroupInfo* infos = static_cast<XanimeGroupInfo*>(pDir[index].mChildren);

                for (int j = 0; j < pDir[index].mSize; j++) {
                    if (infos[j].mAnimationHash == hash) {
                        return &infos[j];
                    }
                }

                return nullptr;
            }
        }
    }

    i = 0;
    u32 hash = MR::getHashCode(pPath);
    const XanimeGroupInfo* result = nullptr;

    while (true) {
        XanimeDirectory* entry = &pDir[i];
        if (entry->mName[0] == '\0') {
            break;
        }

        switch (entry->mDirectoryType) {
        case XanimeDirectory::Recursive:
            result = getGroupInfo(pPath, static_cast<XanimeDirectory*>(entry->mChildren));
            break;
        case XanimeDirectory::Leaf:
            XanimeGroupInfo* subInfo = static_cast<XanimeGroupInfo*>(entry->mChildren);
            for (int j = 0; j < entry->mSize; j++) {
                if (subInfo[j].mAnimationHash == hash) {
                    return &static_cast<XanimeGroupInfo*>(entry->mChildren)[j];
                }
            }

            break;
        }

        if (result != nullptr) {
            return result;
        }

        i++;
    }

    return nullptr;
}

u32 XanimeResourceTable::getIndexFromHash(u32 hash) const {
    if (mSortTable != nullptr) {
        uintptr_t index;
        if (mSortTable->search(hash, &index)) {
            return index;
        }
    }

    return -1;
}

u32 XanimeResourceTable::getGroupIndex(const char* pTarget) const {
    u32 hash = MR::getHashCode(pTarget);

    switch (mHasGroupTable) {
    case 0:
        return -1;
    case 1:
#ifdef GALAXY_NATIVE
    {
#endif
        uintptr_t position = -1;
        if (mSortTable->search(hash, &position)) {
            return position;
        }

#ifdef GALAXY_NATIVE
    }
#endif
    case 2:
        break;
    }

    return -1;
}

XanimeGroupInfo* XanimeResourceTable::getGroupInfoFromHash(u32 hash) const {
    s32 index = getIndexFromHash(hash);
    if (index == -1) {
        return 0;
    }

    return &mGroupInfos[index];
}

u32 XanimeResourceTable::getSingleIndex(const char* pTarget) const {
    if (mBckTables == nullptr) {
        return -1;
    }

    u32 hash = MR::getHashCode(pTarget);

    for (int i = 0; i < mAmountOfBckTables; i++) {
        if (hash == mBckTables[i].mAnimationHash) {
            return i;
        }
    }

    return -1;
}

u32 XanimeResourceTable::getSimpleIndex(const char* pTarget) const {
    u32 hash = MR::getHashCode(pTarget);

    for (int i = 0; i < mAmountOfSimpleGroupInfos; i++) {
        if (hash == mSimpleGroupInfos[i].mAnimationHash) {
            return i;
        }
    }

    return -1;
}

void XanimeResourceTable::createSortTable() {
    mSortTable = new HashSortTable(mAmountOfGroupInfos);
    for (int i = 0; i < mAmountOfGroupInfos; i++) {
        mSortTable->add(mGroupInfos[i].mAnimationHash, i);
    }

    mSortTable->sort();
}

void* XanimeResourceTable::findResMotion(const char* pTarget) const {
    const char* pName = swapBckName(pTarget, mSwapTable);
    return (!mResourceHolder->mMotionResTable->isExistRes(pName)) ? nullptr : mResourceHolder->mMotionResTable->getRes(pName);
}

const char* XanimeResourceTable::findStringMotion(const char* pArg) const {
    return mResourceHolder->mMotionResTable->findFileInfo(pArg)->mName;
}

const char* XanimeResourceTable::swapBckName(const char* pOriginal, XanimeSwapTable* pTable) const {
    if (pTable == nullptr) {
        return pOriginal;
    }

    int i = 0;

    while (true) {
        if (pTable[i].mOriginal[0] == '\0') {
            return pOriginal;
        }

        if (MR::strcasecmp(pTable[i].mOriginal, pOriginal) == 0) {
            return pTable[i].mSwapped;
        }

        i++;
    }
}

bool XanimeResourceTable::search(XanimeBckTable** pArray, const char* pTarget, u32 sizeOfArrayElements) const {
    if (pArray == nullptr) {
        return false;
    }

    if (pArray[0] == nullptr) {
        return false;
    }

    while (true) {
        const char* string = pArray[0]->mAnimationName;
        if (strcmp(string, pTarget) == 0) {
            return true;
        }

        if (pArray[0]->mAnimationName[0] == '\0') {
            return false;
        }

        pArray[0] = reinterpret_cast< XanimeBckTable* >(reinterpret_cast< u8* >(pArray[0]) + sizeOfArrayElements);
    }
}
