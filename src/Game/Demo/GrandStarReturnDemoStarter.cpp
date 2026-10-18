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

namespace {
    const char* const cDemoMovePartName = "移動";
    const char* const cDemoWaitPartName = "ウェイト→コア突入";
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
    NEW_NERVE(GrandStarReturnDemoStarterNrvRevival, GrandStarReturnDemoStarter, Revival);
    NEW_NERVE(GrandStarReturnDemoStarterNrvStageResult, GrandStarReturnDemoStarter, StageResult);
    NEW_NERVE(GrandStarReturnDemoStarterNrvFadeOut, GrandStarReturnDemoStarter, FadeOut);
    NEW_NERVE(GrandStarReturnDemoStarterNrvWaitDemoEnd, GrandStarReturnDemoStarter, WaitDemoEnd);
};  // namespace NrvGrandStarReturnDemoStarter


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
    if (isNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvMove)) ||
        isNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvFlyWait))) {
        MR::setPlayerBaseMtx(mPrevTransform);

        mTransform.setInline(mPrevTransform);
    }

    if (!MR::isDead(mPowerStar)) {
        PowerStar::requestPointLightAtResultSequence(mPowerStar);
    }
}

void GrandStarReturnDemoStarter::calcOffsetStarToCore(TVec3f* pOffset) const {
    TVec3f namePos;
    TVec3f jointPos;

    MR::findNamePos("コア中心", &namePos, nullptr);
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

    offset.scale(100.0f * frame, mDistanceToCore);
    newPosition.add(rPosition, offset);
    mTransform.setTrans(newPosition);
}


inline bool isActiveGrandStarReturnDemoIndex(s32 index) {
    return AstroDemoFunction::getActiveGrandStarReturnDemoIndex() == index;
}


void GrandStarReturnDemoStarter::exeFlyWait() {
    if (MR::isFirstStep(this)) {
        updateRailMoveEndDir();
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

void GrandStarReturnDemoStarter::exeRushToCore() {
    TVec3f position;
    mPrevTransform.getTrans(position);

    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("ResultFlyGrandStarRush");
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
        setNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvRevival));
    }
}



void GrandStarReturnDemoStarter::exeFadeOut() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeFade(60);
    }

    if (!MR::isWipeActive()) {
        MR::endMultiActorCamera(this, mActorCameraInfo, "リザルト", false);
        setNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvWaitDemoEnd));
    }
}

void GrandStarReturnDemoStarter::exeWaitDemoEnd() {
    if (MR::isFirstStep(this)) {
        if (isActiveGrandStarReturnDemoIndex(0)) {
            MR::startStageBGM("MBGM_GALAXY_24", false);
        } else if (isActiveGrandStarReturnDemoIndex(1)) {
            MR::startStageBGM("STM_ASTRO_OUT", false);
        } else if (isActiveGrandStarReturnDemoIndex(2)) {
            MR::startStageBGM("STM_ASTRO_OUT_2", false);
        } else if (isActiveGrandStarReturnDemoIndex(3)) {
            MR::startStageBGM("STM_ASTRO_OUT_2", false);
        } else {
            MR::startStageBGM("STM_ASTRO_OUT_3", false);
        }

        MR::resumeTimeKeepDemo(this);
    }

    if (!MR::isTimeKeepDemoActive()) {
        kill();
    }
}

GrandStarReturnDemoStarter::~GrandStarReturnDemoStarter() {
}

void GrandStarReturnDemoStarter::tryStartStageResult(const char* pDemoName) {
    if (MR::isDemoPartLastStep(pDemoName)) {
        if (GameSequenceFunction::hasStageResultSequence()) {
            mStageResultInformer->appear();
            MR::requestMovementOn(mStageResultInformer);
        }

        MR::pauseTimeKeepDemo(this);
        setNerve(GET_NERVE(GrandStarReturnDemoStarter, GrandStarReturnDemoStarterNrvStageResult));
    }
}

// NONMATCHING (not restored): exeMove needs MR::startLevelSoundPlayer(const char*) (1-arg); PlayerUtil.hpp declares (const char*, s32, s32)
