#include "Game/Demo/DemoDirector.hpp"
#include "Game/Util.hpp"
#include "Game/Demo/DemoCastGroupHolder.hpp"
#include "Game/Demo/DemoExecutor.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/Demo/DemoSimpleCastHolder.hpp"
#include "Game/Demo/StoryBookDemo.hpp"
#include "Game/NPC/EventDirector.hpp"
#include "Game/Screen/PlayerActionGuidance.hpp"
#include "Game/Demo/DemoStartRequestHolder.hpp"
#include "Game/Demo/DemoStartRequestUtil.hpp"
#include "Game/Player/MarioAccess.hpp"
#include "Game/Scene/SceneNameObjMovementController.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/ResourceHolder.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"

DemoDirector::DemoDirector(const char* pName)
    : NameObj(pName), mIsActive(), mUnused13(), mExecutor(), mIsEndPending(), mCastGroupHolder(), mSimpleCastHolder(new DemoSimpleCastHolder(512, 64, 128)), mResourceHolder(),
      mStartRequestHolder(new DemoStartRequestHolder()), mStarterObj(), mDemoName(), mSceneStopType(-1), mIsCinemaFrame(true) {
    MR::connectToScene(this, MR::MovementType_DemoDirector, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);

    mCastGroupHolder = new DemoCastGroupHolder();
    mCastGroupHolder->initWithoutIter();

    mCastSubGroupHolder = new DemoCastGroupHolder();
    mCastSubGroupHolder->initWithoutIter();

    mResourceHolder = DemoFunction::loadDemoArchive();
}

void DemoDirector::movement() {
    if (mExecutor != nullptr && !mIsEndPending) {
        mExecutor->movement();
    }

    if (tryStartDemoRequested()) {
        return;
    }

    if (!mIsEndPending) {
        return;
    }

    if (!MR::isCameraInterpolatingNearlyEnd()) {
        return;
    }

    mIsEndPending = false;

    doDemoEndRequest();

    if (mIsActive) {
        bool isGrandStarDemo;
        if (MR::isDemoActive("グランドスター後発進デモ") || MR::isDemoActive("グランドスター帰還後（ワールド１）") ||
            MR::isDemoActive("グランドスター帰還後（ワールド２）") || MR::isDemoActive("グランドスター帰還後（ワールド３）") ||
            MR::isDemoActive("グランドスター帰還後（ワールド４）") || MR::isDemoActive("グランドスター帰還後（ワールド５）") ||
            MR::isDemoActive("グランドスター帰還[ワールド２以降]") || MR::isDemoActive("グランドスター帰還") ||
            MR::isDemoActive("クッパオープニングデモ") || MR::isDemoActive("クッパダウンデモ") || MR::isDemoActive("クッパ最終戦開始デモ") ||
            MR::isDemoActive("クッパ最終戦終了デモ") || MR::isDemoActive("パワースター帰還[アイテム惑星]")) {
            isGrandStarDemo = true;
        } else {
            isGrandStarDemo = false;
        }

        if (isGrandStarDemo) {
            MR::setSeVolumeSetLevel(11, 20);
        }
    }
}




void DemoDirector::endDemo(NameObj* pParam1, const char* pParam2, bool param3) {
    if (DemoStartRequestUtil::isExistStartDemoRequest(mStartRequestHolder)) {
        MR::sendMsgToAllLiveActor(ACTMES_END_DEMO, nullptr);

        mExecutor = nullptr;

        doDemoEndRequest();
        startDemoRequested();
    } else if (param3 && !MR::isCameraInterpolatingNearlyEnd() && mSceneStopType != 3) {
        mIsEndPending = true;

        MR::sendMsgToAllLiveActor(ACTMES_END_DEMO, nullptr);

        mExecutor = nullptr;

        MR::getSceneObj< SceneNameObjMovementController >(SceneObj_SceneNameObjMovementController)->requestStopSceneOverwrite(pParam1);
    } else {
        MR::sendMsgToAllLiveActor(ACTMES_END_DEMO, nullptr);

        mExecutor = nullptr;

        doDemoEndRequest();
    }
}

const char* DemoDirector::getCurrentDemoName() const {
    if (mIsActive) {
        return mDemoName;
    }

    return nullptr;
}


bool DemoDirector::isExistTimeKeepDemo(const char* pParam1) const {
    for (int i = 0; i < mCastGroupHolder->getObjNum(); i++) {
        if (MR::isName(mCastGroupHolder->getCastGroup(i), pParam1)) {
            return true;
        }
    }

    return false;
}

bool DemoDirector::registerDemoCast(LiveActor* pActor, const JMapInfoIter& rIter) {
    if (!MR::isValidInfo(rIter)) {
        return false;
    }

    s32 demoGroupId = -1;

    if (!MR::getJMapInfoDemoGroupID(rIter, &demoGroupId)) {
        return false;
    }

    JMapIdInfo idInfo = JMapIdInfo(MR::getJMapInfoDemoGroupID(rIter), rIter);

    if (mCastGroupHolder->tryRegisterDemoActor(pActor, rIter, idInfo)) {
        return true;
    }

    return mCastSubGroupHolder->tryRegisterDemoActor(pActor, rIter, idInfo);
}

bool DemoDirector::registerDemoCast(LiveActor* pActor, const char* pParam2, const JMapInfoIter& rIter) {
    if (mCastGroupHolder->tryRegisterDemoActor(pActor, pParam2, rIter)) {
        return true;
    }

    return mCastSubGroupHolder->tryRegisterDemoActor(pActor, pParam2, rIter);
}

void DemoDirector::registerDemoSimpleCast(LiveActor* pActor) {
    mSimpleCastHolder->registerActor(pActor);
}

void DemoDirector::registerDemoSimpleCast(LayoutActor* pActor) {
    mSimpleCastHolder->registerActor(pActor);
}

void DemoDirector::registerDemoSimpleCast(NameObj* pObj) {
    mSimpleCastHolder->registerNameObj(pObj);
}

void DemoDirector::movementOnAllCasts() {
    mSimpleCastHolder->movementOnAllCasts();
}

void DemoDirector::movementOffAllCasts() {
    mSimpleCastHolder->movementOffAllCasts();
}

ResourceHolder* DemoDirector::getDemoResourceHolder(s32 index) const {
    return mResourceHolder[index];
}

bool DemoDirector::tryStartDemoRequested() {
    if (!mStartRequestHolder->isExistRequest()) {
        return false;
    }

    if (!MR::canStartDemo()) {
        return false;
    }

    startDemoRequested();
    return true;
}

void DemoDirector::startDemo(NameObj* pObj, const char* pDemoName, bool isCinemaFrame, s32 type) {
    mIsActive = true;
    mUnused13 = false;
    mStarterObj = pObj;
    mDemoName = pDemoName;
    mSceneStopType = type;
    mIsCinemaFrame = isCinemaFrame;

    if (!MR::isStoryBookDemoActive()) {
        mSimpleCastHolder->movementOnAllCasts();
    }

    MR::initStarPieceGetCSSound();
}

void DemoDirector::startDemoRequested() {
    DemoStartRequestUtil::startDemo(mStartRequestHolder);
    const DemoStartInfo* pInfo = mStartRequestHolder->getCurrentInfo();
    NameObj* pStarter = DemoStartRequestUtil::getDemoStarter(*pInfo);
    startDemo(pStarter, pInfo->mDemoName, pInfo->mFrameType == DemoStartInfo::CinemaFrameType_0, pInfo->mSceneStopType);
    DemoStartRequestUtil::popStartDemoRequest(mStartRequestHolder);
}

void DemoDirector::doDemoEndRequest() {
    MR::getSceneNameObjMovementController()->requestPlaySceneFor(static_cast< MR::MovementControlType >(mSceneStopType), mStarterObj);

    if (MR::isStoryBookTurnPaused()) {
        MR::stopStoryBookDemoCasts();
    }

    MR::activateDefaultGameLayout();

    if (MR::isExistSceneObj(SceneObj_ScoreAttackAccessor)) {
        MR::showScoreAttackResults();
    }

    if (MR::isExistSceneObj(SceneObj_PlayerActionGuidance)) {
        MR::showPlayerActionGuidance();
    }

    if (MR::isExistSceneObj(SceneObj_EventDirector)) {
        MR::showCometTimer();
    }

    MR::endStarPointerMode(mStarterObj);

    if (mIsCinemaFrame) {
        MR::tryFrameToScreenCinemaFrame();
    }

    if (static_cast< u32 >(mSceneStopType - 2) <= 1) {
        MarioAccess::fn_803B6830(false);
    }

    mIsActive = false;
}

void DemoDirector::startDemoProgrammable(NameObj* pObj, const char* pDemoName, bool isCinemaFrame, s32 type) {
    startDemo(pObj, pDemoName, isCinemaFrame, type);
}

void DemoDirector::startDemoTimeKeep(NameObj* pObj, const char* pDemoName, s32 type, bool isCinemaFrame, const char* pPartName) {
    startDemo(pObj, pDemoName, isCinemaFrame, type);
    startDemoExecutor(pObj, pDemoName, type, pPartName);
}

void DemoDirector::startDemoExecutor(NameObj* pObj, const char* pDemoName, s32 type, const char* pPartName) {
    mExecutor = DemoFunction::findDemoExecutor(pDemoName);

    if (pPartName != nullptr) {
        mExecutor->startPart(pObj, pDemoName, pPartName, type);
    } else {
        mExecutor->start(pObj, pDemoName, type);
    }
}
