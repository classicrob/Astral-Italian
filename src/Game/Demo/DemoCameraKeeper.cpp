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

namespace {
    char sSheetName[] = "Camera";
};  // namespace
