#include "Game/NameObj/NameObjGroup.hpp"
#include "Game/Util/ObjUtil.hpp"

NameObjGroup::NameObjGroup(const char* pGroupName, int maxCount) : NameObj(pGroupName) {
    mObjNumMax = 0;
    mObjNum = 0;
    mObjArray = 0;
    initObjArray(maxCount);
}

NameObjGroup::~NameObjGroup() {
    delete[] mObjArray;
}

void NameObjGroup::registerObj(NameObj* pObj) {
    mObjArray[mObjNum] = pObj;
    mObjNum++;
}

void NameObjGroup::clear() {
    for (s32 i = 0; i < mObjNumMax; i++) {
        mObjArray[i] = nullptr;
    }

    mObjNum = 0;
}

bool NameObjGroup::isFull() const {
    return mObjNum == mObjNumMax;
}

void NameObjGroup::pauseOffAll() const {
    for (s32 i = 0; i < mObjNum; i++) {
        MR::requestMovementOn(mObjArray[i]);
    }
}

void NameObjGroup::initObjArray(int maxCount) {
    mObjNumMax = maxCount;
    mObjArray = new NameObj*[maxCount];

    for (s32 i = 0; i < mObjNumMax; i++) {
        mObjArray[i] = 0;
    }
}
