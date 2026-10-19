#include "Game/Demo/DemoCameraKeeper.hpp"
#include "Game/Util.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/Demo/DemoCameraFunction.hpp"
#include "Game/Demo/DemoExecutor.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <cstdio>

namespace {
    extern char sSheetName[];
};  // namespace

DemoCameraInfo::DemoCameraInfo()
    : mPartName(), mCameraTargetName(), mCameraTargetCastID(-1), mAnimCameraName(), mAnimCameraStartFrame(-1), mAnimCameraEndFrame(-1),
      mIsContinuous(), mEventCameraName(), mCameraInfo(), mTargetActor() {
}

void DemoCameraKeeper::initCast(LiveActor* pActor, const JMapInfoIter& rIter) {
    for (s32 i = 0; i < mInfoNum; i++) {
        DemoCameraInfo* curInfo = &mInfos[i];
        if (!DemoFunction::isTargetDemoCast(pActor, rIter, curInfo->mCameraTargetName, curInfo->mCameraTargetCastID)) {
            continue;
        }

        curInfo->mTargetActor = pActor;
        initActorCamera(curInfo, rIter);
    }
}









DemoCameraKeeper::DemoCameraKeeper(DemoExecutor* pExecutor, const JMapInfoIter& rIter) : mExecutor(pExecutor), mInfoNum(), mInfos(), mCurrentIndex(-1), mCurrentInfo() {
    JMapInfo* map = nullptr;
    mInfoNum = DemoFunction::createSheetParser(mExecutor, ::sSheetName, &map);

    mInfos = new DemoCameraInfo[mInfoNum];

    s32 executorNameLen = strlen(mExecutor->mName);

    for (s32 i = 0; i < mInfoNum; i++) {
        DemoCameraInfo* curInfo = &mInfos[i];
        map->getValue< const char* >(i, "PartName", &curInfo->mPartName);
        map->getValue< const char* >(i, "CameraTargetName", &curInfo->mCameraTargetName);
        map->getValue< s32 >(i, "CameraTargetCastID", &curInfo->mCameraTargetCastID);
        map->getValue< const char* >(i, "AnimCameraName", &curInfo->mAnimCameraName);
        map->getValue< s32 >(i, "AnimCameraStartFrame", &curInfo->mAnimCameraStartFrame);
        map->getValue< s32 >(i, "AnimCameraEndFrame", &curInfo->mAnimCameraEndFrame);
        s32 continuous = -1;
        map->getValue< s32 >(i, "IsContinuous", &continuous);

        curInfo->mIsContinuous = (continuous == true);
        DemoCameraFunction::setStringNullIfEmpty(&curInfo->mCameraTargetName);
        DemoCameraFunction::setStringNullIfEmpty(&curInfo->mAnimCameraName);

        s32 animNameLen = executorNameLen + strlen(curInfo->mPartName) + 3;

        char* aninName = new char[animNameLen];

        snprintf(aninName, animNameLen, "%s[%s]", mExecutor->mName, curInfo->mPartName);

        curInfo->mEventCameraName = aninName;
        if (curInfo->mCameraTargetName == nullptr || DemoCameraFunction::isCameraTargetMario(curInfo)) {
            initActorCamera(curInfo, rIter);
        }
    }
}

void DemoCameraKeeper::start() {
    mCurrentIndex = 0;
}

void DemoCameraKeeper::update() {
    if (mCurrentIndex < 0 || mInfoNum <= mCurrentIndex) {
        return;
    }

    for (s32 i = 0; i < mInfoNum; i++) {
        executeType(&mInfos[i]);
    }
}

void DemoCameraKeeper::end() {
    endCurrentCamera();
    mCurrentIndex = -1;
}

void DemoCameraKeeper::endForce() {
    if (mCurrentInfo != nullptr && mCurrentInfo->mAnimCameraName != nullptr) {
        char animName[256];
        MR::removeExtensionString(animName, sizeof(animName), mCurrentInfo->mAnimCameraName);
        MR::endAnimCamera(mCurrentInfo->mTargetActor, mCurrentInfo->mCameraInfo, animName, 0, true);
    }

    endCurrentCamera();
    mCurrentIndex = -1;
}

void DemoCameraKeeper::initActorCamera(DemoCameraInfo* pInfo, const JMapInfoIter& rIter) {
    pInfo->mCameraInfo = new ActorCameraInfo(-1, MR::getPlacedZoneId(rIter));
    MR::declareEventCamera(pInfo->mCameraInfo, pInfo->mEventCameraName);

    if (pInfo->mAnimCameraName != nullptr) {
        char animName[256];
        DemoCameraFunction::makeAnimCameraName(animName, sizeof(animName), pInfo);
        MR::initAnimCamera(pInfo->mTargetActor, pInfo->mCameraInfo, animName);
    }
}

void DemoCameraKeeper::executeType(const DemoCameraInfo* pInfo) {
    if (MR::isDemoPartFirstStep(pInfo->mPartName)) {
        endCurrentCamera();
        mCurrentInfo = pInfo;
        executeFirst(pInfo);
    } else if (MR::isDemoPartLastStep(pInfo->mPartName)) {
        executeLast(mCurrentInfo);
        mCurrentIndex++;
    }
}

void DemoCameraKeeper::executeFirst(const DemoCameraInfo* pInfo) {
    if (pInfo->mTargetActor != nullptr) {
        if (pInfo->mAnimCameraName != nullptr) {
            char animName[256];
            MR::removeExtensionString(animName, sizeof(animName), pInfo->mAnimCameraName);
            MR::startAnimCameraTargetSelf(pInfo->mTargetActor, pInfo->mCameraInfo, animName, pInfo->mAnimCameraStartFrame, false, 1.0f);
        } else {
            MR::startEventCamera(pInfo->mCameraInfo, pInfo->mEventCameraName, CameraTargetArg(pInfo->mTargetActor), -1);
        }
    } else if (DemoCameraFunction::isCameraTargetMario(pInfo)) {
        MR::startEventCameraTargetPlayer(pInfo->mCameraInfo, pInfo->mEventCameraName, -1);
    } else {
        MR::startEventCameraNoTarget(pInfo->mCameraInfo, pInfo->mEventCameraName, -1);
    }
}

void DemoCameraKeeper::executeLast(const DemoCameraInfo* pInfo) {
    if (pInfo->mAnimCameraName != nullptr) {
        char animName[256];
        MR::removeExtensionString(animName, sizeof(animName), pInfo->mAnimCameraName);
        MR::endAnimCamera(pInfo->mTargetActor, pInfo->mCameraInfo, animName, pInfo->mAnimCameraEndFrame, true);
    }
}

void DemoCameraKeeper::endCurrentCamera() {
    if (mCurrentInfo != nullptr) {
        MR::endEventCamera(mCurrentInfo->mCameraInfo, mCurrentInfo->mEventCameraName, mCurrentInfo->mIsContinuous != 0, -1);
    }

    mCurrentInfo = nullptr;
}

namespace {
    char sSheetName[] = "Camera";
};  // namespace
