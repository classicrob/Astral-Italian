#include "Game/NPC/Penguin.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/MapObj/JetTurtle.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/NPCUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    static const f32 sDistanceCall = 350.0f;
    // static const _32 sCallFrameStart =
    // static const _32 sCallFrameEnd =
    static const f32 sSpeedWalk = 1.5f;
    static const f32 sSpeedDash = 6.0f;
    static const f32 sBlendDash = 0.08f;
    static const f32 sSpeedSwim = 5.0f;
    static const f32 sSpeedSwimTurtle = 10.0f;
    static const s32 sStepToDiveMin = 120;
    static const s32 sStepToDiveMax = 300;
    // static const _32 sDistanceDiveMessage =
    // static const _32 sDistanceDiveTalk =
};  // namespace

namespace NrvPenguin {
    NEW_NERVE(PenguinNrvWait, Penguin, Wait);
    NEW_NERVE(PenguinNrvReaction, Penguin, Reaction);
    NEW_NERVE(PenguinNrvDive, Penguin, Dive);
    NEW_NERVE(PenguinNrvFlow, Penguin, Flow);
};  // namespace NrvPenguin

RemovableTurtle::RemovableTurtle(LiveActor* pHost, bool isGolden) {
    if (isGolden) {
        mShellModel = new TurtlePartsModel(pHost, "ペンギン用ゴールデンコウラ", "KouraShine", nullptr,
                                           MR::DrawBufferType_NoSilhouettedMapObjStrongLight, false);
        mJetTurtle = new GoldenTurtle("ゴールデン甲羅");
    } else {
        mShellModel = new PartsModel(pHost, "ペンギン用コウラ", "Koura", nullptr, MR::DrawBufferType_NoSilhouettedMapObjStrongLight, false);
        mJetTurtle = new JetTurtle("ジェット亀さん");
        MR::startBrk(mShellModel, "Color");
        MR::setBrkFrameAndStop(mShellModel, 0.0f);
    }

    mShellModel->mCheckHiddenHost = true;
    mShellModel->initFixedPosition(TVec3f(-5.85f, -68.0f, 30.0f), TVec3f(0.0f, 0.0f, 0.0f), nullptr);
    mShellModel->initWithoutIter();
    mShellModel->makeActorDead();

    mJetTurtle->initWithoutIter();
    mJetTurtle->_E0 = false;
    mJetTurtle->makeActorDead();
}

bool RemovableTurtle::isPullRange() const {
    return MR::isNearPlayer(mShellModel, 450.0f);
}

bool RemovableTurtle::tryRemove() {
    if (MR::isDead(mJetTurtle) && !MR::isPlayerCarryAny()) {
        mShellModel->makeActorDead();
        MR::setPosition(mJetTurtle, mShellModel->mPosition);
        mJetTurtle->appearAndTryTaken();
        return true;
    }

    return false;
}

bool RemovableTurtle::tryAttach() {
    if (MR::isDead(mJetTurtle)) {
        mShellModel->appear();
        return true;
    }
    return false;
}

void Penguin::control() {
    NPCActor::control();
    MR::changeShowModelFlagSyncNearClipping(this, 200.0f);
}

Penguin::Penguin(const char* pName) : NPCActor(pName), mBehavior(Behavior_Default), mStepToDive() {
}

void Penguin::init(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg0NoInit(rIter, &mBehavior);
    NPCActorCaps caps("Penguin");
    caps.setDefault();
    caps.mMessageOffset = TVec3f(0.0f, 150.0f, 0.0f);
    caps.mScaleJointName = "CenterJoint";
    caps.mReactionNerve = GET_NERVE(Penguin, PenguinNrvReaction);
    initialize(rIter, caps, "Penguin", nullptr, nullptr);
    setNerve(GET_NERVE(Penguin, PenguinNrvWait));

    if (MR::isConnectedWithRail(rIter)) {
        MR::moveCoordToNearestPos(this, mPosition);
        mPosition.set(MR::getRailPos(this));
        MR::onCalcShadowAll(this);
    }

    if (MR::tryRegisterDemoCast(this, rIter)) {
        DemoFunction::registerDemoTalkMessageCtrl(this, mMsgCtrl);
    }

    if (mBehavior == Behavior_SwimTurtle) {
        mTurtle = new RemovableTurtle(this, false);
        mTurtle->tryAttach();
    }

    s32 color = 0;
    MR::getJMapInfoArg7NoInit(rIter, &color);
    MR::startBrk(this, "ColorChange");
    MR::setBrkFrameAndStop(this, color);

    mSpinAction=cNPCSpinDefault; mTrampledAction=cNPCTrampledDefault;
    mPointingAction=cNPCPointingDefault; mReactionAction=cNPCReactionDefault;
    mUseReactionActions = false;
    mSpinAttackRange = ::sDistanceCall;

    switch (mBehavior) {
    case Behavior_Default:
    case Behavior_Stand:
        mParam.setMoveAction("WaitNormal", "Turn");
        mParam.setTalkAction("TalkNormal", "TalkTurn");
        setTalkAction("WalkNormal");
        mRailMoveSpeed = ::sSpeedWalk;
        mRailTalkAnimRate = 2.0f;  // talkBckRate?
        mFollowRailOnGround = true;  // follow rail on ground
        mUseReactionActions = true;
        break;
    case Behavior_Sit:
        mParam.setMoveTalkNoTurnAction("SitDown", "SitDownTalk");
        break;
    case Behavior_Swim:
        mParam.setMoveAction("SwimWaitNormal", "SwimWaitTalk");
        setTalkAction("SwimNormal");
        mRailMoveSpeed = ::sSpeedSwim;
        break;
    case Behavior_SwimSurface:
        mParam.setMoveAction("SwimWaitSurfaceNormal", "SwimSurfaceTalk");
        setTalkAction("SwimSurfaceNormal");
        mRailMoveSpeed = ::sSpeedSwim;
        break;
    case Behavior_Dive:
        mParam.setSingleAction("SwimWaitSurfaceNormal");
        break;
    case Behavior_SwimTurtle:
        mParam.setSingleAction("SwimTurtleTalk");
        setTalkAction("SwimTurtle");
        mRailMoveSpeed = ::sSpeedSwimTurtle;
        MR::onCalcGravity(this);
        break;
    case Behavior_Dash:
        mParam.setMoveAction("WaitNormal", "Turn");
        mParam.setTalkAction("TalkNormal", "TalkTurn");
        setTalkAction("DashA");
        mRailMoveSpeed = ::sSpeedDash;
        mRailTurnRate = ::sBlendDash;
        mFollowRailOnGround = true;  // follow rail on ground
        mUseReactionActions = true;
        break;
    default:
        break;
    }
}

// NONMATCHING: vector temporary register allocation.
void Penguin::initAfterPlacement() {
    NPCActor::initAfterPlacement();

    switch (mBehavior) {
    case Behavior_SwimSurface:
    case Behavior_Dive: {
        MR::calcGravity(this);
        MR::turnQuatYDirRate(&mOrientationQuat, mOrientationQuat, -mGravity, 1.0f);

        TVec3f pos;
        TVec3f grav = mGravity;
        Triangle triangle;
        if (MR::getFirstPolyOnLineToWaterSurface(&pos, &triangle, mPosition - grav * 100.0f, grav * 500.0f)) {
            mPosition.set(pos);
        }

        setInitPose();
    }
    default:
        break;
    }
}

void Penguin::exeReaction() {
    if (mTrampledStart) {
        MR::startActionSound(this, "SmPenguinTrampled");
        MR::startActionSound(this, "SvPenguinSTrampled");
    }

    if (isPointingSe()) {
        MR::startActionSound(this, "SvPenguinSPointed2p");
        MR::startDPDHitSound();
    }

    if (mSpinAttackedStart) {
        MR::startActionSound(this, "SvPenguinSSpinHit");
    }

    if (mReactionStart) {
        MR::startActionSound(this, "SvPenguinSStarPiece");
    }

    if (MR::tryStartReactionAndPopNerve(this)) {
        return;
    }
}

void Penguin::exeWait() {
    if (MR::isFirstStep(this)) {
        mStepToDive = MR::getRandom(::sStepToDiveMin, ::sStepToDiveMax);
        MR::onCalcShadowOneTimeAll(this);
    }

    if (MR::tryStartReactionAndPushNerve(this, GET_NERVE(Penguin, PenguinNrvReaction))) {
        return;
    }

    switch (mBehavior) {
    case Behavior_Default:
    case Behavior_Stand:
    case Behavior_Dash:
        MR::tryChangeTalkActionRandom(this, "TalkNormal", "TalkA", "TalkB");
    default:
        break;
    }

    MR::tryTalkNearPlayerAndStartMoveTalkAction(this);

    if (mBehavior == Behavior_SwimTurtle) {
        MR::startActionSound(this, "SmLvPenguinSwimJet");
    }

    if (mBehavior == Behavior_Dive && MR::isStep(this, mStepToDive)) {
        setNerve(GET_NERVE(Penguin, PenguinNrvDive));
    }
}

void Penguin::exeDive() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::startAction(this, "SwimDive");
        MR::startActionSound(this, "SvPenguinSDive");
    }

    if (MR::isActionEnd(this)) {
        MR::validateClipping(this);
        setNerve(GET_NERVE(Penguin, PenguinNrvWait));
    }
}

void Penguin::exeFlow() {
    if (MR::isFirstStep(this)) {
        mRailMoveAction = "SwimNormal";
        mRailTalkAction = "SwimNormal";
        mRailMoveSpeed = ::sSpeedSwim;
    }

    MR::tryTalkNearPlayerAndStartMoveTalkAction(this);

    if (mTurtle->tryAttach()) {
        mParam.setSingleAction("SwimTurtleTalk");
        mRailMoveAction = "SwimTurtle";
        mRailTalkAction = "SwimTurtle";
        mRailMoveSpeed = ::sSpeedSwimTurtle;
        setNerve(GET_NERVE(Penguin, PenguinNrvWait));
    }
}

void Penguin::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        tryReleaseTurtle();
    }

    NPCActor::attackSensor(pSender, pReceiver);
}

bool Penguin::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgSpinStormRange(msg)) {
        if (tryReleaseTurtle()) {
            return true;
        }
    }

    return NPCActor::receiveOtherMsg(msg, pSender, pReceiver);
}

bool Penguin::tryReleaseTurtle() {
    if (mBehavior == Behavior_SwimTurtle && isNerve(GET_NERVE(Penguin, PenguinNrvWait)) && mTurtle->isPullRange() && mTurtle->tryRemove()) {
        setNerve(GET_NERVE(Penguin, PenguinNrvFlow));
        return true;
    }

    return false;
}
