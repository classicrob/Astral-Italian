#include "Game/Demo/DemoStartRequestHolder.hpp"
#include "Game/Demo/DemoStartRequestUtil.hpp"
#include "Game/NameObj/NameObj.hpp"
#include "Game/Util/StringUtil.hpp"
#include <revolution/types.h>

#ifdef GALAXY_NATIVE
// Clang requires explicit specializations to be declared before their first use.
template <>
MR::FixedRingBuffer< const DemoStartInfo*, 16 >::iterator::iterator(const DemoStartInfo** pHead, const DemoStartInfo** pTail, const DemoStartInfo** pEnd);
template <>
void MR::FixedRingBuffer< const DemoStartInfo*, 16 >::push_back(const DemoStartInfo* const& rValue);
template <>
void MR::FixedRingBuffer< const DemoStartInfo*, 16 >::iterator::operator++();
#endif

DemoStartInfo::DemoStartInfo() {
    mLiveActor = nullptr;
    mLayoutActor = nullptr;
    mNerveExecutor = nullptr;
    mNameObj = nullptr;
    mRelatedObj = nullptr;
    mDemoExecutor = nullptr;
    mDemoName = nullptr;
    mSubName = nullptr;
    mNerve = nullptr;
    mSceneStopType = 0;
    mDemoType = DemoType_0;
    mFrameType = CinemaFrameType_0;
    mPointerType = StarPointerType_0;
    mDeleteEffectType = DeleteEffectType_0;
}

DemoStartInfo& DemoStartInfo::operator=(const DemoStartInfo& rOther) {
    mLiveActor = rOther.mLiveActor;
    mLayoutActor = rOther.mLayoutActor;
    mNerveExecutor = rOther.mNerveExecutor;
    mNameObj = rOther.mNameObj;
    mRelatedObj = rOther.mRelatedObj;
    mDemoExecutor = rOther.mDemoExecutor;
    mDemoName = rOther.mDemoName;
    mSubName = rOther.mSubName;
    mNerve = rOther.mNerve;
    mSceneStopType = rOther.mSceneStopType;
    mDemoType = rOther.mDemoType;
    mFrameType = rOther.mFrameType;
    mPointerType = rOther.mPointerType;
    mDeleteEffectType = rOther.mDeleteEffectType;
    return *this;
}

void DemoStartRequestHolder::pushRequest(LiveActor* pActor, const char* pName) {
    const DemoStartInfo* pInfo = find(pActor, pName);
    if (pInfo == nullptr) {
        pInfo = findEmpty();
    }

    mRequestBuffer.push_back(pInfo);
}

void DemoStartRequestHolder::pushRequest(LayoutActor* pActor, const char* pName) {
    const DemoStartInfo* pInfo = find(pActor, pName);
    if (pInfo == nullptr) {
        pInfo = findEmpty();
    }

    mRequestBuffer.push_back(pInfo);
}

void DemoStartRequestHolder::pushRequest(NerveExecutor* pExecutor, const char* pName) {
    const DemoStartInfo* pInfo = find(pExecutor, pName);
    if (pInfo == nullptr) {
        pInfo = findEmpty();
    }

    mRequestBuffer.push_back(pInfo);
}

void DemoStartRequestHolder::pushRequest(NameObj* pObj, const char* pName) {
    const DemoStartInfo* pInfo = find(pObj, pName);
    if (pInfo == nullptr) {
        pInfo = findEmpty();
    }

    mRequestBuffer.push_back(pInfo);
}

void DemoStartRequestHolder::popRequest() {
    DemoStartInfo** pIter = mStartInfos;
    const DemoStartInfo** pCurReqPtr = mRequestBuffer.mHead.mHead;
    DemoStartInfo** pEnd = &mStartInfos[mNumInfos];

    goto check;
    do {
        pIter++;
    check:
        if (pIter == pEnd) {
            break;
        }
    } while (*pIter != *pCurReqPtr);

    if (pIter != pEnd) {
        DemoStartInfo* pFound = *pIter;
        DemoStartInfo emptyInfo;
        *pFound = emptyInfo;
    }

    if (mRequestBuffer.mCount != 0) {
        ++mRequestBuffer.mHead;
        mRequestBuffer.mCount--;
    }
}

bool DemoStartRequestHolder::isExistRequest() const {
    return mRequestBuffer.mCount != 0;
}

const DemoStartInfo* DemoStartRequestHolder::getCurrentInfo() const {
    if (mRequestBuffer.mCount == 0) {
        return nullptr;
    }

    return *mRequestBuffer.mHead.mHead;
}

void DemoStartRequestHolder::registerStartDemoInfo(const DemoStartInfo& rInfo) {
    DemoStartInfo* pEmpty = findEmpty();
    *pEmpty = rInfo;
}

DemoStartInfo* DemoStartRequestHolder::find(const LiveActor* pActor, const char* pName) const {
    for (DemoStartInfo* const* pIter = mStartInfos; pIter != &mStartInfos[mNumInfos]; pIter++) {
        if ((*pIter)->mLiveActor == pActor && MR::isEqualString((*pIter)->mDemoName, pName)) {
            return *pIter;
        }
    }

    return nullptr;
}

DemoStartInfo* DemoStartRequestHolder::find(const LayoutActor* pActor, const char* pName) const {
    for (DemoStartInfo* const* pIter = mStartInfos; pIter != &mStartInfos[mNumInfos]; pIter++) {
        if ((*pIter)->mLayoutActor == pActor && MR::isEqualString((*pIter)->mDemoName, pName)) {
            return *pIter;
        }
    }

    return nullptr;
}

DemoStartInfo* DemoStartRequestHolder::find(const NerveExecutor* pExecutor, const char* pName) const {
    for (DemoStartInfo* const* pIter = mStartInfos; pIter != &mStartInfos[mNumInfos]; pIter++) {
        if ((*pIter)->mNerveExecutor == pExecutor && MR::isEqualString((*pIter)->mDemoName, pName)) {
            return *pIter;
        }
    }

    return nullptr;
}

DemoStartInfo* DemoStartRequestHolder::find(const NameObj* pObj, const char* pName) const {
    for (DemoStartInfo* const* pIter = mStartInfos; pIter != &mStartInfos[mNumInfos]; pIter++) {
        if ((*pIter)->mNameObj == pObj && MR::isEqualString((*pIter)->mDemoName, pName)) {
            return *pIter;
        }
    }

    return nullptr;
}

DemoStartInfo* DemoStartRequestHolder::findEmpty() const {
    DemoStartInfo* pFound;
    for (DemoStartInfo* const* pIter = mStartInfos; pIter != &mStartInfos[mNumInfos]; pIter++) {
        if (DemoStartRequestUtil::isEmpty(*pIter)) {
            pFound = *pIter;
            DemoStartInfo emptyInfo;
            *pFound = emptyInfo;
            return *pIter;
        }
    }

    return nullptr;
}

DemoStartRequestHolder::DemoStartRequestHolder() : mNumInfos(), mRequestBuffer(mRequestBuffer.mBuffer, mRequestBuffer.mBuffer) {
    mProxyObj = new NameObj("代理人");
    for (u32 i = 0; i < ARRAY_SIZE(mStartInfos); i++) {
        DemoStartInfo* pInfo = new DemoStartInfo();
        s32 idx = mNumInfos;
        mNumInfos = idx + 1;
        mStartInfos[idx] = pInfo;
    }
}

template <>
MR::FixedRingBuffer< const DemoStartInfo*, 16 >::iterator::iterator(const DemoStartInfo** pHead, const DemoStartInfo** pTail, const DemoStartInfo** pEnd) {
    mHead = pHead;
    mTail = pTail;
    mEnd = pEnd;
}

template <>
void MR::FixedRingBuffer< const DemoStartInfo*, 16 >::push_back(const DemoStartInfo* const& rValue) {
    if ((u32)(mCount + 1) >= 16) {
        return;
    }

    *mEnd.mHead = rValue;
    ++mEnd;
    mCount++;
}

template <>
void MR::FixedRingBuffer< const DemoStartInfo*, 16 >::iterator::operator++() {
    const DemoStartInfo** pEnd = mEnd;
    mHead++;

    if (pEnd <= mHead) {
        mHead = mTail;
    }
}
