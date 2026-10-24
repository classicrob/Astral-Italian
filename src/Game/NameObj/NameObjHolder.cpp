#include "Game/NameObj/NameObjHolder.hpp"
#include "Game/NameObj/NameObj.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/HashUtil.hpp"
#include <algorithm>
#ifdef GALAXY_NATIVE
#include <petari/game_compat.hpp>
#endif
#include <cstring>

namespace {
    struct equal_fullname {
        equal_fullname(const char* pName) : mHash(MR::getHashCode(pName)), mName(pName) {
        }

        bool operator()(const NameObj* pObj) const {
            return strcmp(pObj->getName(), mName) == 0;
        }

        /* 0x00 */ u32 mHash;
        /* 0x04 */ const char* mName;
    };
};  // namespace

NameObjHolder::NameObjHolder(int a1) : mObjs(0), mMaxObjs(0), mObjCount(0) {
    mObjs = new NameObj*[a1];
    mMaxObjs = a1;
}

void NameObjHolder::add(NameObj* pObj) {
    u32 count = mObjCount;
    mObjCount = count + 1;
    mObjs[count] = pObj;
}

void NameObjHolder::suspendAllObj() {
    for (s32 i = 0; i < mObjCount; i++) {
        MR::requestMovementOff(mObjs[i]);
    }
}

void NameObjHolder::resumeAllObj() {
    for (s32 i = 0; i < mObjCount; i++) {
        MR::requestMovementOn(mObjs[i]);
    }
}

void NameObjHolder::syncWithFlags() {
    callMethodAllObj(&NameObj::syncWithFlags);
}

void NameObjHolder::callMethodAllObj(func functionPtr) {
    std::for_each(mObjs, mObjs + mObjCount, std::mem_func(functionPtr));
}

NameObj* NameObjHolder::find(const char* pName) {
    NameObj** pCached = std::find_if(mObjArray2.begin(), mObjArray2.end(), equal_fullname(pName));
    if (pCached != mObjArray2.end()) {
        NameObj* pObj = *pCached;
        mObjArray2.erase(pCached);
        mObjArray2.insert(mObjArray2.begin(), pObj);
        return pObj;
    }

    NameObj** pFound = std::find_if(mObjs, mObjs + mObjCount, equal_fullname(pName));
    if (pFound == mObjs + mObjCount) {
        return nullptr;
    }

    NameObj* pObj = *pFound;
    if (mObjArray2.size() >= 0x10) {
        mObjArray2.pop_back();
    }

    mObjArray2.insert(mObjArray2.begin(), pObj);
    return pObj;
}

void NameObjHolder::clearArray() {
    mObjCount = 0;
    mObjArray2.clear();
}
