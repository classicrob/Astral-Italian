// NONMATCHING: callback temporary allocation, equivalent boolean code generation, and literal/nerve addressing remain.
#include "Game/NPC/ScoreAttackMan.hpp"
#include "Game/NPC/PlayAttackMan.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/MapObj/ActorStatePlayerPosReset.hpp"
#include "Game/MapObj/ScoreAttack.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include "Game/Util.hpp"

namespace {
    const char* sCallDemo="スコアアタック協会呼び止め";
    const char* sResultDemo="スコアアタック協会結果発表";
}
namespace NrvScoreAttackMan {
    NEW_NERVE(WaitCall,ScoreAttackMan,Wait);
    NEW_NERVE(CallNear,ScoreAttackMan,Talk);
    NEW_NERVE(CallFar,ScoreAttackMan,Talk);
    NEW_NERVE(ResetCall,ScoreAttackMan,ResetPlayer);
    NEW_NERVE(StartCamera,ScoreAttackMan,StartCamera);
    class Wait : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<ScoreAttackMan*>(spine->mExecutor)->NPCActor::exeWait(); }
        static Wait sInstance;
    };
    Wait Wait::sInstance;
    class WaitResultDemo : public Nerve {
    public:
        virtual void execute(Spine* spine) const;
        static WaitResultDemo sInstance;
    };
    WaitResultDemo WaitResultDemo::sInstance;
    NEW_NERVE(ResetResult,ScoreAttackMan,ResetPlayer);
    NEW_NERVE(TalkResult,ScoreAttackMan,TalkResult);
    class TakeOutStar : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static TakeOutStar sInstance;
    };
    TakeOutStar TakeOutStar::sInstance;
    inline void WaitResultDemo::execute(Spine* spine) const {
        ScoreAttackMan* actor=static_cast<ScoreAttackMan*>(spine->mExecutor);
        if(MR::tryStartDemoWithoutCinemaFrame(actor,sResultDemo)) actor->setNerve(&ResetResult::sInstance);
    }
}

ScoreAttackMan::ScoreAttackMan(const char* name) : NPCActor(name),mResetState(nullptr),mTalkCtrl(nullptr),mCameraInfo(nullptr),mTakeOutStar(nullptr),mNearRadius(800.0f),mRestartInfo(nullptr),mIsInRange(false) {}

void ScoreAttackMan::init(const JMapInfoIter& iter) {
    PlayAttackMan::initialize(this,iter,false);
    mRestartInfo=new JMapIdInfo(1,iter);
    initTalkCtrlDirect(iter,"ScoreAttackMan_ScoreAttack",TVec3f(0.0f,150.0f,0.0f),nullptr);
    MR::registerBranchFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&ScoreAttackMan::branchFunc));
    MR::registerEventFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&ScoreAttackMan::eventFunc));
    MR::registerAnimeFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&ScoreAttackMan::animeFunc));
    mTalkCtrl=MR::createTalkCtrlDirect(this,iter,"ScoreAttackMan_Call",TVec3f(0.0f,150.0f,0.0f),nullptr);
    mResetState=new ActorStatePlayerPosReset(this,"マリオ位置",false);
    MR::tryRegisterNamePosLinkObj(this,iter,"マリオ位置");
    MR::getJMapInfoArg0NoInit(iter,&mNearRadius);
    mTakeOutStar=new TakeOutStar(this,"TakeOutStar","TakeOutStarScoreAttackMan",&NrvScoreAttackMan::TakeOutStar::sInstance,nullptr);
    MR::declarePowerStar(this);
    MR::initMultiActorCamera(this,iter,&mCameraInfo,"呼び止め");
    MR::initMultiActorCamera(this,iter,&mCameraInfo,"再セット会話");
    MR::invalidateClipping(this);
    MR::registerScoreAttackMan(this);
    if(GameSequenceFunction::isResultMiss() && MR::isMessageRead(mMsgCtrl)) {
        mWaitNerve=&NrvScoreAttackMan::Wait::sInstance;
        setNerve(&NrvScoreAttackMan::StartCamera::sInstance);
    } else {
        mWaitNerve=&NrvScoreAttackMan::WaitCall::sInstance;
        setNerve(&NrvScoreAttackMan::WaitCall::sInstance);
    }
    MR::initUseStageSwitchWriteB(this,iter);
    makeActorAppeared();
}

void ScoreAttackMan::startResult() {
    tryPullNullNerve();
    MR::invalidateClipping(this);
    setNerve(&NrvScoreAttackMan::WaitResultDemo::sInstance);
}
bool ScoreAttackMan::hasDeclaredPowerStar() {
    return MR::hasPowerStarInCurrentStageWithDeclarer(mName,-1);
}
void ScoreAttackMan::exeWait() {
    if(MR::isFirstStep(this)) mIsInRange=true;
    MR::tryStartTurnAction(this);
    MR::tryStartReactionAndPushNerve(this,mReactionNerve);
}
void ScoreAttackMan::exeTalk() {
    if(MR::isFirstStep(this)) {
        if(isNerve(&NrvScoreAttackMan::CallFar::sInstance) && mCameraInfo) MR::startMultiActorCameraTargetSelf(this,mCameraInfo,"呼び止め",-1);
        MR::startAction(this,"Talk");
    }
    MR::tryStartTurnAction(this);
    MR::tryTalkForce(mTalkCtrl);
    if(MR::isStep(this,60)) setNerve(&NrvScoreAttackMan::ResetCall::sInstance);
}
void ScoreAttackMan::exeStartCamera() {
    MR::endStartPosCamera();
    if(mCameraInfo) MR::startMultiActorCameraTargetSelf(this,mCameraInfo,"再セット会話",-1);
    setNerve(&NrvScoreAttackMan::Wait::sInstance);
}
void ScoreAttackMan::exeResetPlayer() {
    if(MR::isFirstStep(this)) { MR::offPlayerControl(); mResetState->appear(); }
    if(mResetState->isResetStart()) {
        if(mCameraInfo && MR::isActiveMultiActorCamera(this,mCameraInfo,"呼び止め")) MR::endMultiActorCamera(this,mCameraInfo,"呼び止め",false,-1);
        if(mCameraInfo) MR::startMultiActorCameraTargetSelf(this,mCameraInfo,"再セット会話",-1);
        MR::killScoreAttackResults();
        turnToPlayer(180.0f);
        MR::requestMovementOnPlayer();
        MR::startBckPlayer("Watch",nullptr);
        if(isNerve(&NrvScoreAttackMan::ResetResult::sInstance)) {
            if(MR::isValidSwitchB(this)) MR::offSwitchB(this);
            MR::tryStartTalkAction(this);
            MR::tryPlayerKillTakingActor();
            MR::killScoreAttackLayouts();
            MR::stopStageBGM(15);
        }
    }
    if(mResetState->update()) {
        MR::turnPlayerToActor(this,180.0f);
        if(isNerve(&NrvScoreAttackMan::ResetResult::sInstance)) {
            MR::endDemo(this,sResultDemo);
            MR::deactivateDefaultGameLayout();
            setNerve(&NrvScoreAttackMan::TalkResult::sInstance);
        } else {
            MR::endDemo(this,sCallDemo);
            MR::deactivateDefaultGameLayout();
            setNerve(&NrvScoreAttackMan::Wait::sInstance);
        }
    }
}
void ScoreAttackMan::exeTalkResult() {
    MR::tryStartTalkAction(this);
    if(MR::tryTalkForceMarioPuppetableAtEnd(mMsgCtrl)) setNerve(mTalkNerve);
}
void ScoreAttackMan::control() {
    NPCActor::control();
    if(mIsInRange && !MR::isStageStateScenarioOpeningCamera() && !MR::isExecScenarioStarter()) {
        bool isFar = !MR::isNearPlayerHorizontal(this, mNearRadius);
        if(isFar && MR::tryStartDemoWithoutCinemaFrame(this,sCallDemo)) {
            mIsInRange=false;
            tryPullNullNerve();
            MR::resetNode(mTalkCtrl);
            setNerve(&NrvScoreAttackMan::CallFar::sInstance);
            return;
        }
        if(MR::isNearPlayerHorizontal(this,240.0f) && MR::tryStartDemoWithoutCinemaFrame(this,sCallDemo)) {
            mIsInRange=false;
            tryPullNullNerve();
            MR::resetAndForwardNode(mTalkCtrl,1);
            setNerve(&NrvScoreAttackMan::CallNear::sInstance);
        }
    }
}
bool ScoreAttackMan::branchFunc(u32 id) {
    if(id==0) return MR::isScoreAttackFinished();
    if(id==1) return MR::isScoreAttackClear();
    if(id==2) { if(MR::isScoreAttackFirstClear()) return false; return MR::isScoreAttackNewRecord(); }
    if(id==3) return MR::isEqualStageName("MokumokuValleyGalaxy");
    if(id==4) return MR::isEqualStageName("HoneyBeeVillageGalaxy");
    return false;
}
bool ScoreAttackMan::eventFunc(u32 id) {
    if(id==0) { GameSequenceFunction::restartCurrentStageAndClearAfterMiss(); return false; }
    if(id==1) { TalkMessageCtrl* ctrl=mMsgCtrl; MR::setMessageArg(ctrl,MR::getScoreAttackScore()); return true; }
    if(id==2) {
        if(mTakeOutStar->takeOut()) {
        if(mCameraInfo && MR::isActiveMultiActorCamera(this,mCameraInfo,"再セット会話")) MR::endMultiActorCamera(this,mCameraInfo,"再セット会話",false,-1);
        MR::endScoreAttack();
        MR::validateClipping(this);
        setNerve(mWaitNerve);
        return true;
        }
        return false;
    }
    if(id==3) {
        if(mCameraInfo && MR::isActiveMultiActorCamera(this,mCameraInfo,"再セット会話")) MR::endMultiActorCamera(this,mCameraInfo,"再セット会話",false,-1);
        MR::setRestartMarioNo(*mRestartInfo);
        MR::validateClipping(this);
        mWaitNerve=&NrvScoreAttackMan::Wait::sInstance;
        MR::startScoreAttack();
        MR::startStageBGM("MBGM_MINI_GAME",false);
        return true;
    }
    if(id==4) { MR::forceKillPlayerByGroundRace(); return true; }
    if(id==5) { TalkMessageCtrl* ctrl=mMsgCtrl; MR::setMessageArg(ctrl,MR::getScoreAttackTargetScore()); return true; }
    return true;
}
bool ScoreAttackMan::animeFunc(u32 id) {
    switch(id) {
    case 0: MR::startAction(this,"Talk"); return true;
    case 1: MR::startAction(this,"TalkOk"); return true;
    case 2: MR::startAction(this,"Jump"); return true;
    default: return true;
    }
}
