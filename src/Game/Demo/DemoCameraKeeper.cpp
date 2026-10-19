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
      mIsContinuous(), _1C(), _20(), _24() {
}

void DemoCameraKeeper::initCast(LiveActor* pActor, const JMapInfoIter& rIter) {
    for (s32 i = 0; i < _4; i++) {
        DemoCameraInfo* curInfo = &_8[i];
        if (!DemoFunction::isTargetDemoCast(pActor, rIter, curInfo->mCameraTargetName, curInfo->mCameraTargetCastID)) {
            continue;
        }

        curInfo->_24 = pActor;
        initActorCamera(curInfo, rIter);
    }
}









DemoCameraKeeper::DemoCameraKeeper(DemoExecutor* pExecutor, const JMapInfoIter& rIter) : mExecutor(pExecutor), _4(), _8(), _C(-1), _10() {
    JMapInfo* map = nullptr;
    _4 = DemoFunction::createSheetParser(mExecutor, ::sSheetName, &map);

    _8 = new DemoCameraInfo[_4];

    s32 executorNameLen = strlen(mExecutor->mName);

    for (s32 i = 0; i < _4; i++) {
        DemoCameraInfo* curInfo = &_8[i];
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

        curInfo->_1C = aninName;
        if (curInfo->mCameraTargetName == nullptr || DemoCameraFunction::isCameraTargetMario(curInfo)) {
            initActorCamera(curInfo, rIter);
        }
    }
}

void DemoCameraKeeper::start() {
    _C = 0;
}

void DemoCameraKeeper::update() {
    if (_C < 0 || _4 <= _C) {
        return;
    }

    for (s32 i = 0; i < _4; i++) {
        executeType(&_8[i]);
    }
}

void DemoCameraKeeper::end() {
    endCurrentCamera();
    _C = -1;
}

void DemoCameraKeeper::endForce() {
    if (_10 != nullptr && _10->mAnimCameraName != nullptr) {
        char animName[256];
        MR::removeExtensionString(animName, sizeof(animName), _10->mAnimCameraName);
        MR::endAnimCamera(_10->_24, _10->_20, animName, 0, true);
    }

    endCurrentCamera();
    _C = -1;
}

void DemoCameraKeeper::initActorCamera(DemoCameraInfo* pInfo, const JMapInfoIter& rIter) {
    pInfo->_20 = new ActorCameraInfo(-1, MR::getPlacedZoneId(rIter));
    MR::declareEventCamera(pInfo->_20, pInfo->_1C);

    if (pInfo->mAnimCameraName != nullptr) {
        char animName[256];
        DemoCameraFunction::makeAnimCameraName(animName, sizeof(animName), pInfo);
        MR::initAnimCamera(pInfo->_24, pInfo->_20, animName);
    }
}

void DemoCameraKeeper::executeType(const DemoCameraInfo* pInfo) {
    if (MR::isDemoPartFirstStep(pInfo->mPartName)) {
        endCurrentCamera();
        _10 = pInfo;
        executeFirst(pInfo);
    } else if (MR::isDemoPartLastStep(pInfo->mPartName)) {
        executeLast(_10);
        _C++;
    }
}

void DemoCameraKeeper::executeFirst(const DemoCameraInfo* pInfo) {
    if (pInfo->_24 != nullptr) {
        if (pInfo->mAnimCameraName != nullptr) {
            char animName[256];
            MR::removeExtensionString(animName, sizeof(animName), pInfo->mAnimCameraName);
            MR::startAnimCameraTargetSelf(pInfo->_24, pInfo->_20, animName, pInfo->mAnimCameraStartFrame, false, 1.0f);
        } else {
            MR::startEventCamera(pInfo->_20, pInfo->_1C, CameraTargetArg(pInfo->_24), -1);
        }
    } else if (DemoCameraFunction::isCameraTargetMario(pInfo)) {
        MR::startEventCameraTargetPlayer(pInfo->_20, pInfo->_1C, -1);
    } else {
        MR::startEventCameraNoTarget(pInfo->_20, pInfo->_1C, -1);
    }
}

void DemoCameraKeeper::executeLast(const DemoCameraInfo* pInfo) {
    if (pInfo->mAnimCameraName != nullptr) {
        char animName[256];
        MR::removeExtensionString(animName, sizeof(animName), pInfo->mAnimCameraName);
        MR::endAnimCamera(pInfo->_24, pInfo->_20, animName, pInfo->mAnimCameraEndFrame, true);
    }
}

void DemoCameraKeeper::endCurrentCamera() {
    if (_10 != nullptr) {
        MR::endEventCamera(_10->_20, _10->_1C, _10->mIsContinuous != 0, -1);
    }

    _10 = nullptr;
}

namespace {
    char sSheetName[] = "Camera";
};  // namespace
