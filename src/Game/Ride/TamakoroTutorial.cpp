#pragma auto_inline off
#include "Game/Ride/TamakoroTutorial.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Ride/Tamakoro.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/PlayerActionGuidance.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/TalkUtil.hpp"

namespace NrvTamakoroTutorial {
    NEW_NERVE(HostTypeNrvFirst, TamakoroTutorial, First);
    NEW_NERVE(HostTypeNrvDenyTalk, TamakoroTutorial, DenyTalk);
    NEW_NERVE(HostTypeNrvFirstForceTalk, TamakoroTutorial, FirstForceTalk);
    NEW_NERVE(HostTypeNrvWaitRideTalk, TamakoroTutorial, WaitRideTalk);
    NEW_NERVE(HostTypeNrvWaitRide, TamakoroTutorial, WaitRide);
    NEW_NERVE(HostTypeNrvWaitRaiseTalk, TamakoroTutorial, WaitRaiseTalk);
    NEW_NERVE(HostTypeNrvWaitRaiseStable, TamakoroTutorial, WaitRaiseStable);
    NEW_NERVE(HostTypeNrvRaiseOk, TamakoroTutorial, RaiseOk);
    NEW_NERVE(HostTypeNrvRaiseOkTalk, TamakoroTutorial, RaiseOkTalk);
    NEW_NERVE(HostTypeNrvFinalTalk, TamakoroTutorial, FinalTalk);
    NEW_NERVE(HostTypeNrvWait, TamakoroTutorial, Wait);
    NEW_NERVE(HostTypeNrvDirectRide, TamakoroTutorial, Wait);
    NEW_NERVE(HostTypeNrvTrampleFront, TamakoroTutorial, Trample);
    NEW_NERVE(HostTypeNrvTrampleBack, TamakoroTutorial, Trample);
    NEW_NERVE(HostTypeNrvRecoverFront, TamakoroTutorial, Recover);
    NEW_NERVE(HostTypeNrvRecoverBack, TamakoroTutorial, Recover);
};  // namespace NrvTamakoroTutorial

namespace {
    static const TVec3f hRaiseAcc = TVec3f(0.0f, -1.0f, 0.0f);
    static const f32 hRaiseCheckDegree = 30.0f;
    static const f32 hDistToStartTutorial = 400.0f;
};  // namespace

TamakoroTutorial::TamakoroTutorial(const char* pName)
    : LiveActor(pName), mHost(), mTalkCtrl(), mTalkCtrlAutomatic(), mPadAccel(0.0f, 0.0f, 0.0f), _A4(), _A5() {
}

void TamakoroTutorial::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "SignBoard", "Tutorial", false);
    MR::addEffectHitNormal(this, nullptr);
    MR::createSceneObj(SceneObj_PlayerActionGuidance);
    MR::getPlayerActionGuidance()->createTamakoroLayout();
    appear();

    s32 omitTutorial = -1;
    MR::getJMapInfoArg0NoInit(rIter, &omitTutorial);

    if (omitTutorial == -1) {
        mTalkCtrl = MR::createTalkCtrlDirect(this, rIter, "TamakoroTutorial", TVec3f(0.0f, 0.0f, 0.0f), nullptr);
        _A5 = false;
        initNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvFirst));
    } else {
        mTalkCtrl = MR::createTalkCtrl(this, rIter, "SignBoard", TVec3f(0.0f, 0.0f, 0.0f), nullptr);
        _A5 = true;
        MR::invalidateHitSensor(this, "push");
        mHost->startRide();
        initNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvWait));
    }

    mTalkCtrlAutomatic = MR::createTalkCtrlDirectOnRootNodeAutomatic(this, rIter, "TamakoroTutorial2nd", TVec3f(0.0f, 0.0f, 0.0f), nullptr);

    MR::setMessageBalloonFollowOffset(mTalkCtrl, TVec3f(0.0f, 180.0f, 0.0f));
    MR::setDistanceToTalk(mTalkCtrl, ::hDistToStartTutorial);
    MR::setMessageBalloonFollowOffset(mTalkCtrlAutomatic, TVec3f(0.0f, 180.0f, 0.0f));
    MR::setDistanceToTalk(mTalkCtrlAutomatic, ::hDistToStartTutorial);
    MR::calcGravity(this);
}

void TamakoroTutorial::initAfterPlacement() {
    if (MR::isOnGameEventFlagTamakoroTutorialAtFirst()) {
        MR::invalidateHitSensor(this, "push");
        mHost->startRide();
    }
}

bool TamakoroTutorial::isNeedTutorial() {
    return isNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvWaitRide));
}

void TamakoroTutorial::requestStart() {
    MR::forwardNode(mTalkCtrl);
    setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvWaitRaiseTalk));
}

void TamakoroTutorial::requestEnd() {
    setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvDirectRide));
}

void TamakoroTutorial::control() {
    MR::getCorePadAcceleration(&mPadAccel, WPAD_CHAN0);
    MR::normalizeOrZero(&mPadAccel);
}

void TamakoroTutorial::exeFirst() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "RevivalFront");
        MR::setBckFrameAndStop(this, MR::getBckFrameMax(this));
    }

    if (!MR::isOnGameEventFlagTamakoroTutorialAtFirst()) {
        f32 distPlayerToTutorial = MR::calcDistanceToPlayer(mPosition);
        f32 distPlayerToTamakoro = MR::calcDistanceToPlayer(mHost->mPosition);

        if (MR::isOnGroundPlayer() && !MR::isExecScenarioStarter() &&
            (distPlayerToTutorial < ::hDistToStartTutorial || distPlayerToTamakoro < ::hDistToStartTutorial)) {
            setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvFirstForceTalk));
        }
    } else {
        if (MR::tryTalkNearPlayerAtEnd(mTalkCtrlAutomatic) && MR::tryTalkSelectLeft(mTalkCtrlAutomatic)) {
            MR::resetNode(mTalkCtrl);
            MR::forwardNode(mTalkCtrl);
            setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvWaitRide));
        }
    }
}

void TamakoroTutorial::exeDenyTalk() {
    if (MR::tryTalkForceAtEnd(mTalkCtrlAutomatic)) {
        MR::resetNode(mTalkCtrlAutomatic);
        setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvFirst));
    }
}

void TamakoroTutorial::exeFirstForceTalk() {
    if (MR::isFirstStep(this)) {
        MR::resetNode(mTalkCtrl);
    }

    if (MR::tryTalkForceAtEnd(mTalkCtrl)) {
        setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvWaitRideTalk));
    }
}

void TamakoroTutorial::exeWaitRideTalk() {
    if (MR::isFirstStep(this)) {
        MR::forwardNode(mTalkCtrl);
    }

    if (MR::tryTalkForceAtEnd(mTalkCtrl)) {
        MR::invalidateHitSensor(this, "push");
        mHost->startRide();
        setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvWaitRide));
    }
}

void TamakoroTutorial::exeWaitRide() {
    if (MR::isFirstStep(this)) {
        MR::forwardNode(mTalkCtrl);
    }

    MR::tryTalkNearPlayer(mTalkCtrl);
}

void TamakoroTutorial::exeWaitRaiseTalk() {
    PlayerActionGuidance* pPlayerActionGuidance = MR::getPlayerActionGuidance();

    pPlayerActionGuidance->mGuidanceState = PlayerActionGuidance::GuidanceState_Raise;
    pPlayerActionGuidance->mForceTamakoroGuidance = true;

    MR::tryTalkForceWithoutDemo(mTalkCtrl);

    if (MR::isGreaterStep(this, 90)) {
        if (MR::isNearAngleDegree(::hRaiseAcc, mPadAccel, ::hRaiseCheckDegree)) {
            MR::startActionSound(this, "SySurfTutorialOk", -1, -1, -1);
            setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvWaitRaiseStable));
        }
    }
}

void TamakoroTutorial::exeWaitRaiseStable() {
    PlayerActionGuidance* pPlayerActionGuidance = MR::getPlayerActionGuidance();

    pPlayerActionGuidance->mGuidanceState = PlayerActionGuidance::GuidanceState_RaiseHold;
    pPlayerActionGuidance->mForceTamakoroGuidance = true;

    MR::tryTalkForceWithoutDemoAtEnd(mTalkCtrl);

    if (!MR::isNearAngleDegree(::hRaiseAcc, mPadAccel, ::hRaiseCheckDegree)) {
        setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvWaitRaiseTalk));
        MR::startActionSound(this, "SySurfTutorialNg", -1, -1, -1);
    } else {
        startTimerSound(getNerveStep(), 1);

        if (MR::isGreaterStep(this, 60)) {
            MR::startActionSound(this, "SySurfTutorialGonext", -1, -1, -1);
            setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvRaiseOk));
        }
    }
}

void TamakoroTutorial::exeRaiseOk() {
    PlayerActionGuidance* pPlayerActionGuidance = MR::getPlayerActionGuidance();

    pPlayerActionGuidance->mGuidanceState = PlayerActionGuidance::GuidanceState_RaiseOK;
    pPlayerActionGuidance->mForceTamakoroGuidance = true;

    if (MR::isGreaterStep(this, 60)) {
        setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvRaiseOkTalk));
    }
}

void TamakoroTutorial::exeRaiseOkTalk() {
    PlayerActionGuidance* pPlayerActionGuidance = MR::getPlayerActionGuidance();

    pPlayerActionGuidance->mGuidanceState = PlayerActionGuidance::GuidanceState_RaiseOK;
    pPlayerActionGuidance->mForceTamakoroGuidance = true;

    if (MR::isFirstStep(this)) {
        MR::forwardNode(mTalkCtrl);
    }

    if (MR::tryTalkForceWithoutDemoAtEnd(mTalkCtrl)) {
        setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvFinalTalk));
    }
}

void TamakoroTutorial::exeFinalTalk() {
    PlayerActionGuidance* pPlayerActionGuidance = MR::getPlayerActionGuidance();

    pPlayerActionGuidance->mGuidanceState = PlayerActionGuidance::GuidanceState_Rotation;
    pPlayerActionGuidance->mForceTamakoroGuidance = true;

    if (MR::isFirstStep(this)) {
        MR::forwardNode(mTalkCtrl);
    }

    if (MR::tryTalkForceWithoutDemoAtEnd(mTalkCtrl)) {
        if (MR::isExistNextNode(mTalkCtrl)) {
            setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvFinalTalk));
        } else {
            mHost->requestTutorialEnd();
            MR::onGameEventFlagTamakoroTutorialAtFirst();
            setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvWait));
        }
    }
}

void TamakoroTutorial::exeWait() {
    if (isNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvDirectRide)) && MR::isFirstStep(this)) {
        mHost->requestTutorialEnd();
        MR::onGameEventFlagTamakoroTutorialAtFirst();
        setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvWait));
    } else if (_A5 && MR::tryTalkNearPlayerAtEnd(mTalkCtrl)) {
        MR::tryForwardNode(mTalkCtrl);
    }
}

void TamakoroTutorial::exeTrample() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OjSignBoardTrampBall", -1, -1, -1);

        if (isNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvTrampleFront))) {
            MR::startAction(this, "DownFront");
        } else {
            MR::startAction(this, "DownBack");
        }
    }

    if (MR::isActionEnd(this) && MR::isGreaterStep(this, 60) && !_A4) {
        if (isNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvTrampleFront))) {
            setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvRecoverFront));
        } else {
            setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvRecoverBack));
        }
    } else {
        _A4 = false;
    }
}

void TamakoroTutorial::exeRecover() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OjSignBoardRecover", -1, -1, -1);

        if (isNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvRecoverFront))) {
            MR::startAction(this, "RevivalFront");
        } else {
            MR::startAction(this, "RevivalBack");
        }
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvWait));
    }
}

void TamakoroTutorial::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvTrampleFront)) && !isNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvTrampleBack))) {
        MR::sendMsgPush(pReceiver, pSender);
    }

    if (MR::isSensorRide(pReceiver)) {
        _A4 = true;
    }
}

bool TamakoroTutorial::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceAttack(msg)) {
        return true;
    }

    if (MR::isMsgPlayerTrample(msg)) {
        if (!MR::isSensorRide(pSender)) {
            MR::startActionSound(this, "OjSignBoardTrampPlr", -1, -1, -1);

            return true;
        }

        if (isNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvWait)) || isNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvRecoverFront)) ||
            isNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvRecoverBack))) {
            const TVec3f* pReceiverPos = MR::getSensorPos(pReceiver);
            TVec3f v1(*MR::getSensorPos(pSender));
            v1 -= *pReceiverPos;
            TVec3f v2;

            MR::calcFrontVec(&v2, this);

            if (0.0f < v2.dot(v1)) {
                setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvTrampleBack));
            } else {
                setNerve(GET_NERVE(TamakoroTutorial, HostTypeNrvTrampleFront));
            }

            MR::emitEffectHitBetweenSensors(this, pSender, pReceiver, 0.0f, nullptr);

            return true;
        }
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        MR::sendMsgAwayJump(pSender, pReceiver);

        return true;
    }

    return false;
}

void TamakoroTutorial::updateHitSensor(HitSensor* pSensor) {
    TVec3f upVec;
    MR::calcUpVec(&upVec, this);
    TVec3f vHostToPlayer(*MR::getPlayerCenterPos());
    vHostToPlayer -= mHost->mPosition;

    f32 dot = upVec.dot(vHostToPlayer);

    MR::setSensorPos(pSensor, mHost->mPosition + upVec * dot);
}

void TamakoroTutorial::startTimerSound(s32 step, s32 param2) {
    if (step == 20) {
        MR::startActionSound(this, "SySurfTutorialTimer", param2, -1, -1);
    }

    if (step == 40) {
        MR::startActionSound(this, "SySurfTutorialTimer2", param2, -1, -1);
    }
}
