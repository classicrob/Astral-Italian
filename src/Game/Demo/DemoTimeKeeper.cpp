#include "Game/Demo/DemoTimeKeeper.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/Demo/DemoExecutor.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"

DemoTimeKeeper::DemoTimeKeeper(const DemoExecutor* pExecutor)
    : mExecutor(pExecutor), mMainPartInfos(), mSubPartInfos(), mNumPartInfos(), mPartStep(-1), mCurrentStep(-1), mCurrentPartIndex(-1), mIsPaused() {
    JMapInfo* pMap = nullptr;
    mNumPartInfos = DemoFunction::createSheetParser(mExecutor, "Time", &pMap);
    mMainPartInfos = new DemoTimePartInfo[mNumPartInfos];

    for (s32 i = 0; i < mNumPartInfos; i++) {
        DemoTimePartInfo* pInfo = &mMainPartInfos[i];
        MR::getCsvDataStrOrNULL(&pInfo->mPartName, pMap, "PartName", i);
        MR::getCsvDataS32(&pInfo->mTotalStep, pMap, "TotalStep", i);

        s32 suspendFlag = 0;
        MR::getCsvDataS32(&suspendFlag, pMap, "SuspendFlag", i);
        pInfo->mSuspendFlag = suspendFlag != 0;
        s32 waitUserInputFlag = 0;
        if (MR::hasCsvItem(pMap, "WaitUserInputFlag")) {
            MR::getCsvDataS32(&waitUserInputFlag, pMap, "WaitUserInputFlag", i);
        }
        pInfo->mWaitUserInputFlag = waitUserInputFlag != 0;
    }
}

DemoTimePartInfo::DemoTimePartInfo() : mPartName(), mTotalStep(1), mSuspendFlag(), mWaitUserInputFlag() {
}

void DemoTimeKeeper::start() {
    mCurrentPartIndex = 0;
    mSubPartInfos = &mMainPartInfos[mCurrentPartIndex];
}

void DemoTimeKeeper::update() {
    if (mIsPaused) {
        if (mPartStep <= 0) {
            mPartStep++;
        }

        if (mCurrentStep <= 0) {
            mCurrentStep++;
        }
    } else {
        mCurrentStep++;
        mPartStep++;

        if (mSubPartInfos->mTotalStep <= mCurrentStep && !mSubPartInfos->mSuspendFlag) {
            mCurrentPartIndex++;

            if (mNumPartInfos > mCurrentPartIndex) {
                mSubPartInfos = &mMainPartInfos[mCurrentPartIndex];
                mCurrentStep = 0;
            }
        }
    }
}

void DemoTimeKeeper::end() {
    mPartStep = -1;
    mCurrentStep = -1;
    mCurrentPartIndex = -1;
    mSubPartInfos = nullptr;
}

bool DemoTimeKeeper::isDemoEnd() const {
    if (mIsPaused) {
        return false;
    }

    if (mSubPartInfos->mSuspendFlag && mSubPartInfos->mTotalStep <= mCurrentStep) {
        return true;
    }

    if (mSubPartInfos->mTotalStep <= mCurrentStep && mNumPartInfos <= mCurrentPartIndex) {
        return true;
    }

    return false;
}

void DemoTimeKeeper::setStartPart(const char* pPartName) {
    setCurrentPart(pPartName);
}

bool DemoTimeKeeper::isExistSuspendFlagCurrentPart() const {
    return mSubPartInfos->mSuspendFlag;
}

bool DemoTimeKeeper::isPartLast() const {
    if (mIsPaused) {
        return false;
    }

    if (mCurrentPartIndex == mNumPartInfos - 1) {
        return true;
    }
    return false;
}

inline s32 DemoTimeKeeper::getPart(const char* pPartName) const {
    for (s32 i = 0; i < mNumPartInfos; i++) {
        if (MR::isEqualString(mMainPartInfos[i].mPartName, pPartName)) {
            return i;
        }
    }

    return -1;
}

void DemoTimeKeeper::setCurrentPart(const char* pPartName) {
    mCurrentPartIndex = getPart(pPartName);
    mSubPartInfos = &mMainPartInfos[mCurrentPartIndex];
}

bool DemoTimeKeeper::isCurrentDemoPartLastStep() const {
    return mSubPartInfos->mTotalStep - 1 <= mCurrentStep;
}

bool DemoTimeKeeper::isTalkWaitUserInput() const {
    return mSubPartInfos->mWaitUserInputFlag;
}

bool DemoTimeKeeper::isLastPart(bool checkPaused) const {
    if (mExecutor->mIsStoryBookStarting) return false;
    if (checkPaused && mIsPaused) return false;
    if (mCurrentPartIndex == mNumPartInfos - 1) {
        return true;
    }
    return false;
}
