#include "Game/LiveActor/DemoObj.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Demo/GrandStarReturnDemoStarter.hpp"
#include "Game/Util.hpp"
#include "Game/Demo/AstroDemoFunction.hpp"
#include "Game/Demo/ReturnDemoRailMove.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/PowerStar.hpp"
#include "Game/Screen/StageResultInformer.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <cstdio>

// Runtime-tunable zero-initialized timing from target .sbss2.
extern s32 gGrandStarReturnBgmStartStep;

namespace {
    const char* const cDemoMovePartName = "移動";
    const char* const cDemoWaitPartName = "ゲートオープン";
    const char* const cGateOpenMessage = "System_ResultGrandStarGateOpen";
};  // namespace

GrandStarReturnDemoStarter::GrandStarReturnDemoStarter(const char* pName)
    : LiveActor(pName), mReturnDemoRailMove(), mStageResultInformer(), mPowerStar(), mDistanceToCore(0.0f, 0.0f, 1.0f), mPowerStarPosition(gZeroVec),
      mActorCameraInfo() {
    mPrevTransform.identity();
    mTransform.identity();
}

// Local prototype: defined in src/Game/System/GameSequenceFunction.cpp, not declared in its header
namespace GameSequenceFunction {
    bool hasStageResultSequence();
};  // namespace GameSequenceFunction

namespace NrvGrandStarReturnDemoStarter {
    NEW_NERVE(GrandStarReturnDemoStarterNrvMove, GrandStarReturnDemoStarter, Move);
    NEW_NERVE(GrandStarReturnDemoStarterNrvFlyWait, GrandStarReturnDemoStarter, FlyWait);
    NEW_NERVE(GrandStarReturnDemoStarterNrvRushToCore, GrandStarReturnDemoStarter, RushToCore);
    NEW_NERVE(GrandStarReturnDemoStarterNrvWaitInformation, GrandStarReturnDemoStarter, WaitInformation);
    NEW_NERVE(GrandStarReturnDemoStarterNrvRevival, GrandStarReturnDemoStarter, Revival);
    NEW_NERVE(GrandStarReturnDemoStarterNrvStageResult, GrandStarReturnDemoStarter, StageResult);
    NEW_NERVE(GrandStarReturnDemoStarterNrvFadeOut, GrandStarReturnDemoStarter, FadeOut);
    NEW_NERVE(GrandStarReturnDemoStarterNrvWaitDemoEnd, GrandStarReturnDemoStarter, WaitDemoEnd);
};  // namespace NrvGrandStarReturnDemoStarter

void GrandStarReturnDemoStarter::exeWaitInformation() {
    if (MR::isDeadInformationMessage()) {
        MR::resumeTimeKeepDemo(this);
        setNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvRevival));
    }
}



void GrandStarReturnDemoStarter::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    mStageResultInformer = new StageResultInformer();
    mStageResultInformer->initWithoutIter();
    mPowerStar = MR::createDemoObj("グランドスターデモモデル", "GrandStar", "Demo", mTransform);
    MR::invalidateClipping(mPowerStar);
    mPowerStar->kill();
    mReturnDemoRailMove = new ReturnDemoRailMove(this, mPowerStar, rIter, true, &mPrevTransform);
    mReturnDemoRailMove->setupPathDrawForGraneStarReturnDemo();
    TVec3f gravity(0.0f, -1.0f, 0.0f);
    mReturnDemoRailMove->mGravity.set(gravity);
    if (MR::tryRegisterDemoCast(this, rIter)) {
        MR::tryRegisterDemoCast(mPowerStar, rIter);
    }
    MR::initMultiActorCamera(this, rIter, &mActorCameraInfo, "移動");
    MR::initMultiActorCamera(this, rIter, &mActorCameraInfo, "ウェイト");
    MR::initMultiActorCamera(this, rIter, &mActorCameraInfo, "リザルト");
    initNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvMove), 0);
    makeActorDead();
}

// NONMATCHING: regalloc

void GrandStarReturnDemoStarter::appear() {
    LiveActor::appear();

    mPowerStar->appear();
    MR::showJointAndChildren(mPowerStar, "PowerStar");

    mReturnDemoRailMove->posToStart();
    mReturnDemoRailMove->start();

    PowerStar::setupColorAtResultSequence(mPowerStar, true);

    setNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvMove));
}

void GrandStarReturnDemoStarter::control() {
    if (isNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvMove))) {
        MR::setPlayerBaseMtx(mPrevTransform);

        mTransform.setInline(mPrevTransform);
    }

    if (!MR::isDead(mPowerStar) && !MR::isHiddenModel(mPowerStar)) {
        PowerStar::requestPointLightAtResultSequence(mPowerStar);
    }
}

void GrandStarReturnDemoStarter::calcOffsetStarToCore(TVec3f* pOffset) const {
    TVec3f namePos;
    TVec3f jointPos;

    MR::findNamePos("ゲート中心", &namePos, nullptr);
    MR::copyJointPos(mPowerStar, "PowerStar", &jointPos);

    pOffset->sub(namePos, jointPos);
}

void GrandStarReturnDemoStarter::emitEffectRush() {
    MR::emitEffect(mPowerStar, "Blur");
    MR::emitEffect(mPowerStar, "Blur1");
    MR::emitEffect(mPowerStar, "Blur2");
    MR::emitEffect(mPowerStar, "Blur3");
    MR::emitEffect(mPowerStar, "Blur4");
    MR::emitEffect(mPowerStar, "Blur5");
}

void GrandStarReturnDemoStarter::updateRushStarPos(const TVec3f& rPosition, s32 frame) {
    TVec3f offset;
    TVec3f newPosition;

    offset.scale(1000.0f * frame, mDistanceToCore);
    newPosition.add(rPosition, offset);
    mTransform.setTrans(newPosition);
}


inline bool isActiveGrandStarReturnDemoIndex(s32 index) {
    return AstroDemoFunction::getActiveGrandStarReturnDemoIndex() == index;
}

void GrandStarReturnDemoStarter::tryStartStageResult(const char* pDemoName) {
    if (MR::isDemoPartLastStep(pDemoName)) {
        mStageResultInformer->appear();
        MR::requestMovementOn(mStageResultInformer);
        MR::pauseTimeKeepDemo(this);
        setNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvStageResult));
    }
}

void GrandStarReturnDemoStarter::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::startMultiActorCameraTargetPlayer(this, mActorCameraInfo, "移動", -1);
    }
    if (MR::isFirstGrandStarReturnBGMStage()) {
        if (MR::isStep(this, gGrandStarReturnBgmStartStep)) {
            MR::startGrandStarReturnBGMIfNotUnlocked();
        }
    } else if (MR::isStep(this, 17)) {
        MR::startGrandStarReturnBGMIfNotUnlocked();
    }
    const char* part = cDemoMovePartName;
    s32 total = MR::getDemoPartTotalStep(part);
    s32 step = MR::getDemoPartStep(part);
    mReturnDemoRailMove->update(step + 1, total);
    MR::startLevelSoundPlayer("SE_PM_LV_SPIN_DRV_FLY", -1);
    TVec3f direction;
    calcOffsetStarToCore(&direction);
    MR::normalize(&direction);
    mReturnDemoRailMove->mForward.set(direction);
    if (MR::isDemoPartLastStep(part)) {
        setNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvFlyWait));
    }
}

void GrandStarReturnDemoStarter::exeFlyWait() {
    if (MR::isFirstStep(this)) {
        TVec3f direction;
        calcOffsetStarToCore(&direction);
        MR::normalize(&direction);
        mReturnDemoRailMove->mForward.set(direction);
        mReturnDemoRailMove->posToEnd();
    }

    if (MR::isStep(this, 60)) {
        emitEffectRush();
    }

    if (MR::isLessStep(this, 60)) {
        MR::startActionSound(mPowerStar, "OjLvRetPowStarFly");
    }

    if (MR::isBckOneTimeAndStoppedPlayer()) {
        setNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvRushToCore));
    }
}

// NONMATCHING: vector operation scheduling

void GrandStarReturnDemoStarter::exeRushToCore() {
    TVec3f position;
    position.set< f32 >(mPrevTransform.mMtx[0][3], mPrevTransform.mMtx[1][3], mPrevTransform.mMtx[2][3]);

    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("ResultFlyGrandStarRush", nullptr);
        MR::startAction(mPowerStar, "ResultFlyGrandStarRush");
        MR::startActionSound(mPowerStar, "OjGndStarRush");

        MR::hideJointAndChildren(mPowerStar, "PowerStar");
        calcOffsetStarToCore(&mDistanceToCore);
        mPowerStarPosition.add(position, mDistanceToCore);
        MR::normalize(&mDistanceToCore);
    }

    updateRushStarPos(position, getNerveStep());
    MR::startActionSound(mPowerStar, "OjLvGndStarRush");

    if (MR::isDemoPartLastStep(::cDemoWaitPartName)) {
        MR::hideModelAndOnCalcAnim(mPowerStar);
        MR::pauseTimeKeepDemo(this);
        MR::appearInformationMessageUp(cGateOpenMessage, true);
        MR::startSubBGM("BGM_JG_APPEAR_PLANET", false);
        MR::setStageBGMVolumeDownDelay(90);
        setNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvWaitInformation));
    }
}

void GrandStarReturnDemoStarter::exeRevival() {
    MR::setSeVolumeSetLevel(10, 30);
    if (MR::isDemoPartFirstStep("リザルト画面")) {
        MR::startMultiActorCameraTargetPlayer(this, mActorCameraInfo, "リザルト", -1);
    }
    tryStartStageResult("リザルト画面");
}

void GrandStarReturnDemoStarter::exeStageResult() {
    MR::setSeVolumeSetLevel(10, 30);
    if (MR::isDead(mStageResultInformer)) {
        if (MR::isWipeBlank()) {
            MR::endMultiActorCamera(this, mActorCameraInfo, "リザルト", false, -1);
            setNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvWaitDemoEnd));
        } else {
            setNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvFadeOut));
        }
    }
}

void GrandStarReturnDemoStarter::exeFadeOut() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeFade(60);
    }

    MR::setSeVolumeSetLevel(10, 30);
    if (!MR::isWipeActive()) {
        MR::endMultiActorCamera(this, mActorCameraInfo, "リザルト", false, -1);
        setNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvWaitDemoEnd));
    }
}

void GrandStarReturnDemoStarter::exeWaitDemoEnd() {
    if (MR::isFirstStep(this)) {
        MR::resumeTimeKeepDemo(this);
    }
    MR::setSeVolumeSetLevel(10, 30);
    if (!MR::isTimeKeepDemoActive()) {
        MR::endStartPosCamera();
        mPowerStar->makeActorDead();
        kill();
    }
}

GrandStarReturnDemoStarter::~GrandStarReturnDemoStarter() {
}
