#include "Game/Animation/XanimeResource.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/System/ResourceInfo.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <cstring>
#include <revolution/types.h>

static bool unknownByte;

XanimeResourceTable::XanimeResourceTable(ResourceHolder* pArg) {
    init();

    _0 = 0;
    mGroupInfos = nullptr;
    mResourceHolder = pArg;
    mAmountOfBckTables = 0;
    mAmountOfGroupInfos = 0;

    mMaxGroupInfoTableSize = 1;

    if (unknownByte != 0) {
        mMaxGroupInfoTableSize = unknownByte;
    }

    _1C.init();

    _1C.mRate = 1.0f;
    _1C.mParent.mAnimationName = "non-group";
    _1C._8 = 1;
    _1C.mBckTableVariant = 1;
}

void XanimeResourceTable::init() {
    _0 = 0;
    mMaxGroupInfoTableSize = 0;
    mAmountOfGroupInfos = 0;
    mAmountOfBckTables = 0;
    mAmountOfSimpleGroupInfos = 0;
    mGroupInfos = nullptr;
    mDirectories = nullptr;

    mResourceHolder = nullptr;
    mSortTable = nullptr;
    mSwapTable = nullptr;
}

const XanimeGroupInfo* XanimeResourceTable::getGroupInfo(const char* pArg) const {
    switch (_0) {
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

    case 2:
        return getGroupInfo(pArg, mDirectories);
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
                return getGroupInfo(&pPath[i + 1], pDir[index].mSubDirectories);

            case XanimeDirectory::Leaf:
                u32 hash = MR::getHashCode(&pPath[i + 1]);
                XanimeGroupInfo* infos = pDir[index].mSubInformations;

                for (int j = 0; j < pDir[index].mSize; j++) {
                    if (infos[j]._40 == hash) {
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
            result = getGroupInfo(pPath, entry->mSubDirectories);
            break;
        case XanimeDirectory::Leaf:
            XanimeGroupInfo* subInfo = entry->mSubInformations;
            for (int j = 0; j < entry->mSize; j++) {
                if (subInfo[j]._40 == hash) {
                    return &entry->mSubInformations[j];
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
        u32 index;
        if (mSortTable->search(hash, &index)) {
            return index;
        }
    }

    return -1;
}

u32 XanimeResourceTable::getGroupIndex(const char* pTarget) const {
    u32 hash = MR::getHashCode(pTarget);

    switch (_0) {
    case 0:
        return -1;
    case 1:
        u32 position;
        if (mSortTable->search(hash, &position)) {
            return position;
        }

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
        if (hash == mSimpleGroupInfos[i]._40) {
            return i;
        }
    }

    return -1;
}

void XanimeResourceTable::createSortTable() {
    mSortTable = new HashSortTable(mAmountOfGroupInfos);
    for (int i = 0; i < mAmountOfGroupInfos; i++) {
        mSortTable->add(mGroupInfos[i]._40, i);
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
