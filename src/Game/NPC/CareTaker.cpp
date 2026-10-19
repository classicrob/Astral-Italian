#include "Game/NPC/CareTaker.hpp"
#include "Game/Map/StartCountdownLayout.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Effect/EffectSystem.hpp"
#include "Game/Effect/EffectSystemUtil.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/NameObj/NameObjArchiveListCollector.hpp"
#include "Game/Screen/BombTimerLayout.hpp"
#include "Game/Util.hpp"


namespace {
    const char* sPlayerPrePosition = "タイムアタック前位置";
    const char* sPlayerPostPosition = "タイムアタック後位置";
    static const s32 sTidyTimeLimit = 30;
    static const s32 sBgmStartStep = 166;
    // static const f32 sTalkRotateDegree = _;
    // static const f32 sTalkCameraScale = _;
    // static const s32 sDirtBtkFrame = _;
    static const s32 sPreRaceWait = 50;
    static const s32 sPstRaceWait = 40;
    // static const s32 sNormalBtkFrame = _;
};  // namespace

namespace NrvCaretaker {
    NEW_NERVE(CaretakerNrvTakeOutStar, Caretaker, TakeOutStar);
    NEW_NERVE(CaretakerNrvReaction, Caretaker, Reaction);
    NEW_NERVE(CaretakerNrvTalk, Caretaker, Talk);
    NEW_NERVE(CaretakerNrvWait, Caretaker, Wait);
    NEW_NERVE(CaretakerNrvPreTalk, Caretaker, PreTalk);
    NEW_NERVE(CaretakerNrvPreWipeOut, Caretaker, PreWipeOut);
    NEW_NERVE(CaretakerNrvPreWipeIn, Caretaker, PreWipeIn);
    NEW_NERVE(CaretakerNrvPreWait, Caretaker, PreWait);
    NEW_NERVE(CaretakerNrvCountDown, Caretaker, CountDown);
    NEW_NERVE(CaretakerNrvTidy, Caretaker, Tidy);
    NEW_NERVE(CaretakerNrvTimeUp, Caretaker, TimeUp);
    NEW_NERVE(CaretakerNrvRunaway, Caretaker, Runaway);
    NEW_NERVE(CaretakerNrvPstWipeOut, Caretaker, PstWipeOut);
    NEW_NERVE(CaretakerNrvPstWipeIn, Caretaker, PstWipeIn);
    NEW_NERVE(CaretakerNrvPstWait, Caretaker, PstWait);
    NEW_NERVE(CaretakerNrvPstTalk, Caretaker, PstTalk);
    NEW_NERVE(CaretakerNrvAppearedStar, Caretaker, AppearedStar);
};  // namespace NrvCaretaker

Caretaker::Caretaker(const char* pName) : NPCActor(pName), mCountdown(nullptr), mBombTimerLayout(nullptr), mObjArg0(-1), mTidyTimeLimit(::sTidyTimeLimit), mResult(-1) {
}

// NONMATCHING: register allocation and member-pointer constant-table addressing differ.
void Caretaker::init(const JMapInfoIter& rIter) {
    s32 arg0=-1;
    MR::getJMapInfoArg0NoInit(rIter,&arg0);
    mObjArg0=arg0;
    NPCActorCaps caps("Caretaker");
    NPCActorItem item("Caretaker");
    caps.setDefault();
    caps.mWaitNerve=GET_NERVE(Caretaker,CaretakerNrvTalk);
    caps.mReactionNerve=GET_NERVE(Caretaker,CaretakerNrvReaction);
    initialize(rIter,caps,"Caretaker",nullptr,nullptr);
    s32 arg3=0;
    MR::getJMapInfoArg3NoInit(rIter,&arg3);
    mBodyColorFrame=arg3;
    MR::startBrk(this,"BodyColor");
    MR::setBrkFrameAndStop(this,mBodyColorFrame);
    mTrampledAction="BTrampled";
    mSpinAction="BSpinHit";
    mReactionAction="BSpinHit";
    mPointingAction="BTalkHelp";
    mParam._14="BWaitStand";
    mParam._18="BWaitStand";
    mRailMoveAction="BWaitRun";
    mRailTalkAction="BRunTalk";
    mParam._0=0;
    mParam._1=1;
    mParam._8=3.0f;
    mRailMoveSpeed=2.0f;
    mRailMoveAccel=0.1f;
    mRailTurnRate=0.05f;
    mFollowRailOnGround=1;
    MR::startBtk(this,"Dirt");
    MR::setBtkFrameAndStop(this,0.0f);
    MR::startBckNoInterpole(this,"Wait");
    s32 arg4=0;
    MR::getJMapInfoArg4NoInit(rIter,&arg4);
    mObjArg4=arg4;
    setAnim(arg4);
    MR::calcAnimDirect(this);
    if(getMsgCtrl()) {
        MR::registerBranchFunc(getMsgCtrl(),TalkMessageFunc(this,&Caretaker::branchFuncComet));
        MR::registerEventFunc(getMsgCtrl(),TalkMessageFunc(this,&Caretaker::eventFuncComet));
    }
    if(static_cast<u32>(mObjArg0)<=2) {
        MR::declarePowerStar(this);
        if(mObjArg0==1) mTakeOutStar=new TakeOutStar(this,"TakeOutStarCaretakerFreeHand","TakeOutStarCaretakerFreeHand",GET_NERVE(Caretaker,CaretakerNrvTakeOutStar),nullptr);
        else mTakeOutStar=new TakeOutStar(this,"TakeOutStarCaretaker","TakeOutStarCaretaker",GET_NERVE(Caretaker,CaretakerNrvTakeOutStar),nullptr);
    }
    if(mObjArg0==0) {
        mSpinAction="SpinHit";
        mReactionAction="SpinHit";
        mTrampledAction="Trampled";
        mPointingAction="TalkAngry";
        mParam._14="Wait";
        mParam._18="Wait";
        mParam._1C="TalkNormal";
        mParam._20="TalkNormal";
        mRailMoveAction="WaitRun";
        mRailTalkAction="WaitRun";
        MR::getNPCItemData(&item,0);
        equipment(item,false);
        if(MR::isExistRail(this)) { MR::moveCoordAndFollowTrans(this); mInitPosition.set(mPosition); }
        MR::needStageSwitchReadA(this,rIter);
        MR::needStageSwitchWriteB(this,rIter);
        MR::getJMapInfoArg1NoInit(rIter,&mTidyTimeLimit);
        mCountdown=new StartCountdownLayout;
        mCountdown->initWithoutIter();
        mBombTimerLayout=new BombTimerLayout;
        mBombTimerLayout->initWithoutIter();
        mBombTimerLayout->setTimeLimit(mTidyTimeLimit*60);
        mBombTimerLayout->kill();
        mGarbageGroup=MR::joinToGroupArray(this,rIter,"ゴミ管理",32);
        MR::enablePlayerResourceFire();
        MR::createSceneObj(0x92);
        if(getMsgCtrl()) {
            MR::registerBranchFunc(getMsgCtrl(),TalkMessageFunc(this,&Caretaker::branchFuncStar));
            MR::registerAnimeFunc(getMsgCtrl(),TalkMessageFunc(this,&Caretaker::animeFunc));
            MR::setMessageArg(getMsgCtrl(),mTidyTimeLimit);
            MR::setDistanceToTalk(getMsgCtrl(),350.0f);
        }
        mCameraInfo=MR::createActorCameraInfo(rIter);
        MR::initActorCamera(this,rIter,&mCameraInfo);
        setNerve(GET_NERVE(Caretaker,CaretakerNrvWait));
    }
}

void Caretaker::initAfterPlacement() {
    if (MR::isExistRail(this)) {
        MR::followRailPoseOnGround(this, 1.0f);
    }
}

void Caretaker::kill() {
    NPCActor::kill();

    if (mBombTimerLayout != nullptr) {
        mBombTimerLayout->kill();
    }
}

void Caretaker::control() {
    NPCActor::control();
}

bool Caretaker::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        mSpinAttacked = 1;

        return false;
    }

    return NPCActor::receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

bool Caretaker::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mObjArg4 == 6) {
        bool isAttack = false;

        if ((MR::isMsgToEnemyAttackBlow(msg) || MR::isMsgToEnemyAttackTrample(msg)) && MR::isSensorType(pSender, 0x30)) {
            isAttack = true;
        }

        if (isAttack) {
            MR::sendMsgToEnemyAttackBlow(pSender, pReceiver);
            mSpinAttacked=true;
            return false;
        }
    }

    return NPCActor::receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

bool Caretaker::branchFuncStar(u32 msg) {
    switch (msg) {
    case 0:
        return MR::isOnSwitchA(this);
    default:
        return false;
    }
}

// NONMATCHING: saved-register/frame-pointer allocation differs; selector branches and signed range tests match.
#pragma global_optimizer off

bool Caretaker::branchFuncComet(u32 msg) {
    s32 purpleCoinNum = MR::getPurpleCoinNum();
    if (msg == 0) goto case0;
    if (msg == 1) goto case1;
    if (msg == 2) goto case2;
    if (msg == 3) goto case3;
    if (msg == 4) goto case4;
    if (msg == 5) goto case5;
    if (msg == 6) goto case6;
    goto fallback;
case0:
    return purpleCoinNum == 0;
case1: {
    bool result = false;
    if (purpleCoinNum >= 1 && purpleCoinNum < 50) {
        result = true;
    }
    return result;
}
case2: {
    bool result = false;
    if (purpleCoinNum >= 50 && purpleCoinNum < 80) {
        result = true;
    }
    return result;
}
case3: {
    bool result = false;
    if (purpleCoinNum >= 80 && purpleCoinNum < 90) {
        result = true;
    }
    return result;
}
case4: {
    bool result = false;
    if (purpleCoinNum >= 90 && purpleCoinNum < 95) {
        result = true;
    }
    return result;
}
case5: {
    bool result = false;
    if (purpleCoinNum >= 95 && purpleCoinNum < 100) {
        result = true;
    }
    return result;
}
case6:
    return purpleCoinNum - 100 == 0;
fallback:
    return false;
}
#pragma global_optimizer on

bool Caretaker::eventFuncComet(u32 msg) {
    if(msg==0) return mTakeOutStar->takeOut();
    if(msg==1) { MR::forceKillPlayerByGroundRace(); return true; }
    if(msg==2) { TalkMessageCtrl* ctrl=getMsgCtrl(); MR::setMessageArg(ctrl,MR::getPurpleCoinNum()); return true; }
    if(msg==3) { GameSequenceFunction::restartCurrentStageAndClearAfterMiss(); return true; }
    if(msg==4) { TalkMessageCtrl* ctrl=getMsgCtrl(); MR::setMessageArg(ctrl,MR::getScoreAttackScore()); return true; }
    return true;
}

bool Caretaker::animeFunc(u32 msg) {
    switch (msg) {
    case 0:
        MR::startAction(this, "TalkNormal");
        break;
    case 1:
        MR::startAction(this, "TalkAngry");
        break;
    case 2:
        MR::startAction(this, "TalkCry");
        break;
    }

    return true;
}

void Caretaker::setAnim(s32 anim) {
    switch (anim) {
    case 0:
        mParam.setTalkAction("BTalkNormal", "BTalkNormal");
        break;
    case 1:
        mParam.setTalkAction("BTalkCry", "BTalkCry");
        break;
    case 2:
        mParam.setTalkAction("BTalkSpin", "BTalkSpin");
        break;
    case 3:
        mParam.setTalkAction("BTalkSurprise", "BTalkSurprise");
        break;
    case 4:
        mParam.setTalkAction("BTalkSpring", "BTalkSpring");
        break;
    case 5:
        mParam.setSingleAction("BTalkHelp");
        break;
    case 6:
        MR::invalidateLodCtrl(this);
        MR::setBtkFrameAndStop(this, 2.0f);
        mParam.setMoveTalkNoTurnAction("BTalkHelp", "BTalkHelp");
        break;
    }
}

void Caretaker::hide() {
    mLodCtrl->invalidate();
    MR::hideModel(this);
    MR::startAction(this, "HideWait");
    MR::invalidateHitSensors(this);

    if (mGoods0 != nullptr) {
        MR::hideModel(mGoods0);
    }

    if (mGoods1 != nullptr) {
        MR::hideModel(mGoods1);
    }
}

void Caretaker::show() {
    MR::showModel(this);
    MR::validateHitSensors(this);
    mLodCtrl->validate();

    if (mGoods0 != nullptr) {
        MR::showModel(mGoods0);
    }

    if (mGoods1 != nullptr) {
        MR::showModel(mGoods1);
    }
}

void Caretaker::exeReaction() {
    if (mTrampledStart) {
        MR::startActionSound(this, "SmNpcTrampledMetal");
        MR::startActionSound(this, "SvCareTakerTrample");
    }

    if (mSpinAttackedStart) {
        MR::startActionSound(this, "SmCaretakerSpinHit");
        MR::startActionSound(this, "SvCareTakerSpinHit");
    }

    if (isPointingSe()) {
        MR::startActionSound(this, "SvCareTakerPointed2p");
        MR::startDPDHitSound();
    }

    if (mReactionStart) {
        MR::startActionSound(this, "SmCaretakerSpinHit");
        MR::startActionSound(this, "SvCareTakerStarPiece");
    }

    if (MR::tryStartReactionAndPopNerve(this)) {
        return;
    }
}

void Caretaker::exeWait() {
    if (MR::tryTalkNearPlayerAndStartMoveTalkAction(this)) {
        MR::startNPCTalkCamera(getMsgCtrl(), getBaseMtx(), 1.3f, -1);
        if(MR::isExistRail(this)) MR::setRailCoordSpeed(this, 0.0f);
        setNerve(GET_NERVE(Caretaker, CaretakerNrvPreTalk));
    } else if (MR::tryStartReactionAndPushNerve(this, GET_NERVE(Caretaker, CaretakerNrvReaction))) {
        return;
    }
}

void Caretaker::exePreTalk() {
    turnToPlayer(mParam._8);
    if (!MR::tryTalkNearPlayerAtEnd(getMsgCtrl())) return;
    MR::setAudioSystemField9A0(90);
    if (MR::tryTalkSelectLeft(getMsgCtrl())) {
        if (MR::tryStartDemoMarioPuppetableWithoutCinemaFrame(this,"ゴミ掃除タイムアタック")) {
            MR::startAction(this,"Wait");
            MR::startBckPlayer("Watch",nullptr);
            setNerve(GET_NERVE(Caretaker,CaretakerNrvPreWipeOut));
            MR::startNPCTalkCamera(getMsgCtrl(),getBaseMtx(),1.0f,0);
        }
    } else { MR::endNPCTalkCamera(false,-1); setNerve(GET_NERVE(Caretaker,CaretakerNrvWait)); }
}

void Caretaker::exePreWipeOut() {
    if (MR::isFirstStep(this)) {
        mResult=MR::getCurrentStageBgmId();
        MR::stopStageBGM(90);
        MR::invalidateClipping(this);
        MR::closeWipeFade(-1);
    }
    MR::setAudioSystemField9A0(90);
    if (MR::isWipeActive()) return;
    if (MR::isExistRail(this)) { MR::setRailCoordSpeed(this,0.0f); MR::setRailCoord(this,0.0f); }
    MR::setDefaultPose(this);
    MR::setPlayerPosOnGroundAndWait(sPlayerPrePosition,false);
    MR::endNPCTalkCamera(false,0);
    MR::callInvalidateClippingAllGroupMember(this);
    mBombTimerLayout->appear();
    mBombTimerLayout->suspend();
    mBombTimerLayout->setTimeLimit(mTidyTimeLimit*60);
    setNerve(GET_NERVE(Caretaker,CaretakerNrvPreWipeIn));
    MR::killPlayerFireBalls();
    if(MR::isPlayerElementModeFire()) MR::forceCurePlayerElementMode();
}

void Caretaker::exePreWipeIn() {
    if(MR::isFirstStep(this)) {
        MR::setExterminationWoodBoxFlag();
        MR::openWipeFade(-1);
        MR::changePlayerModeFire();
        MR::startSubBGM("BGM_MINIGAME_START",false);
        hide();
    }
    MR::setAudioSystemField9A0(90);
    if(!MR::isWipeActive()) setNerve(GET_NERVE(Caretaker,CaretakerNrvPreWait));
}

void Caretaker::exePreWait() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "MorphTimeAttack", -1, -1, -1);
        MR::endDemo(this, "ゴミ掃除タイムアタック");
        MR::offPlayerControl();
        MR::onSwitchB(this);
    }
    MR::setAudioSystemField9A0(0x5A);

    if (MR::isGreaterStep(this, ::sPreRaceWait)) {
        setNerve(GET_NERVE(Caretaker, CaretakerNrvCountDown));
    }
}

void Caretaker::exeCountDown() {
    if(MR::isFirstStep(this)) mCountdown->appear();
    if(mCountdown->isStart()) {
        MR::changePlayerModeFire();
        MR::startSubBGM("BGM_FIRE_B",false);
        MR::onPlayerControl(true);
        setNerve(GET_NERVE(Caretaker,CaretakerNrvTidy));
    }
}

void Caretaker::exeTidy() {
    if(MR::isFirstStep(this)) mBombTimerLayout->resume();
    if(MR::isPlayerInRush() || MR::isPlayerRecovering()) {
        mBombTimerLayout->kill(); MR::stopStageBGM(60); setNerve(GET_NERVE(Caretaker,CaretakerNrvRunaway)); return;
    }
    if(!MR::isPlayerDead()) MR::isPlayerConfrontDeath();
    if(MR::canStartDemo() && (mBombTimerLayout->isReadyToTimeUp() || MR::isOnSwitchA(this))) {
        if(MR::isOnSwitchA(this) && MR::isPlayerElementModeFire()) MR::changePlayerModeFire();
        if(MR::tryStartDemoMarioPuppetableWithoutCinemaFrame(this,"ゴミ掃除タイムアタック")) setNerve(GET_NERVE(Caretaker,CaretakerNrvTimeUp));
    }
}

void Caretaker::exeTimeUp() {
    if (MR::isFirstStep(this)) {
        MR::killPlayerFireBalls();

        if (MR::isOnSwitchA(this)) {
            MR::startActionSound(this, "SyTotalComplete", -1, -1, -1);
            MR::stopStageBGM(10);
        } else {
            MR::startActionSound(this, "TimeUp", -1, -1, -1);
            MR::stopStageBGM(60);
        }
    }

    MR::killPlayerFireBalls();

    if (MR::isGreaterStep(this, 120)) {
        setNerve(GET_NERVE(Caretaker, CaretakerNrvPstWipeOut));
    }
}

void Caretaker::exeRunaway() {
    if(MR::isPlayerInRush() || MR::isPlayerRecovering()) return;
    MR::offSwitchB(this);
    MR::validateClipping(this);
    MR::callAppearAllGroupMember(this);
    MR::callValidateClippingAllGroupMember(this);
    MR::clearExterminationWoodBoxFlag();
    MR::resetNode(getMsgCtrl());
    setNerve(GET_NERVE(Caretaker,CaretakerNrvWait));
    show();
}

void Caretaker::exePstWipeOut() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeFade(-1);
    }

    if (MR::isWipeActive()) {
        return;
    }

    MR::offSwitchB(this);

    if (MR::isOnSwitchA(this)) {
        MR::callKillAllGroupMember(this);
    } else {
        MR::callAppearAllGroupMember(this);
    }

    MR::callValidateClippingAllGroupMember(this);
    MR::Effect::forceDeleteAllEmitters(MR::getEffectSystem());
    MR::tryPlayerKillTakingActor();
    MR::setPlayerPosOnGroundAndWait(sPlayerPostPosition,false);
    MR::startBckPlayer("Watch", nullptr);

    if (MR::isPlayerElementModeFire()) {
        MR::forceCurePlayerElementMode();
    }

    show();
    setNerve(GET_NERVE(Caretaker, CaretakerNrvPstWipeIn));
}

void Caretaker::exePstWipeIn() {
    if(MR::isFirstStep(this)) {
        mBombTimerLayout->kill();
        MR::openWipeFade(-1);
        if(mResult!=-1) MR::startStageBGM(static_cast<u32>(mResult),false);
    }
    if(MR::isWipeActive()) return;
    MR::startAction(this,"TalkNormal");
    MR::validateClipping(this);
    setNerve(GET_NERVE(Caretaker,CaretakerNrvPstWait));
}

void Caretaker::exePstTalk() {
    if (!MR::tryTalkForceWithoutDemoMarioPuppetableAtEnd(mMsgCtrl)) {
        return;
    }

    MR::endDemo(this, "ゴミ掃除タイムアタック");
    MR::endNPCTalkCamera(false, -1);
    MR::clearExterminationWoodBoxFlag();

    if (MR::isOnSwitchA(this)) {
        setNerve(GET_NERVE(Caretaker, CaretakerNrvAppearedStar));
    } else {
        setNerve(GET_NERVE(Caretaker, CaretakerNrvWait));
    }
}

void Caretaker::updateCounterSE() {
    if (MR::isPlayerDead()) {
        return;
    }

    if (MR::isPlayerConfrontDeath()) {
        return;
    }

    u32 restTime = mBombTimerLayout->getRestTime();

    if (restTime == 0) {
        MR::startSystemSE("SE_SY_E3_TIMER_TIME_UP");
    } else if (restTime <= 120) {
        if (restTime % 60 == 0) {
            MR::startSystemSE("SE_SY_E3_TIMER_COUNT_4");
        }
    } else if (restTime <= 360) {
        if (restTime % 60 == 0) {
            MR::startSystemSE("SE_SY_E3_TIMER_COUNT_2");
        }
    } else if (restTime <= mBombTimerLayout->mDangerTransFrame) {
        if (restTime % 60 == 0) {
            MR::startSystemSE("SE_SY_E3_TIMER_COUNT_1");
        }
    }
}

void Caretaker::exeTakeOutStar() {
}

void Caretaker::exeTalk() {
    MR::tryTalkNearPlayerAndStartMoveTalkAction(this);
    MR::tryStartReactionAndPushNerve(this, GET_NERVE(Caretaker, CaretakerNrvReaction));
}

void Caretaker::exePstWait() {
    if (MR::isGreaterEqualStep(this, ::sPstRaceWait)) {
        setNerve(GET_NERVE(Caretaker, CaretakerNrvPstTalk));
    }
}

void Caretaker::exeAppearedStar() {
    MR::tryTalkNearPlayerAndStartTalkAction(this);
    MR::tryStartReactionAndPushNerve(this, GET_NERVE(Caretaker, CaretakerNrvReaction));
}
