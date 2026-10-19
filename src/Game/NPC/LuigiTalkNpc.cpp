#include "Game/NPC/LuigiTalkNpc.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"

namespace {
    const char* sIntrusiveMessage="LuigiIntrusively000";
    const char* sIntrusiveDemo="でしゃばりルイージ終了";
    const char* sAttentionDemo="顔惑星注目会話イベント301";
}
namespace NrvLuigiTalkNpc {
    NEW_NERVE(LuigiTalkNpcNrvTurnPlayer,LuigiTalkNpc,TurnPlayer);
    NEW_NERVE(LuigiTalkNpcNrvInformation,LuigiTalkNpc,Information);
}
LuigiTalkNpc::LuigiTalkNpc(const char* name) : NPCActor(name),mUnusedCounter(0),mHideAfterIntrusiveUnlock(false) {}
LuigiTalkNpc::~LuigiTalkNpc() {}
// NONMATCHING: callback evaluation scheduling and local relocations differ.
void LuigiTalkNpc::init(const JMapInfoIter& iter) {
    NPCActorCaps caps("LuigiTalkNpc");
    caps.setDefault();
    caps.mDropItemOnSupportTicoSpin=true;
    mParam._18="Turn"; mParam._14="Wait"; mParam._1C="Talk"; mParam._20="Turn";
    if(MR::isObjectName(iter,"LuigiIntrusively") && !MR::isExistMessageId(iter)) { caps.mUseDirectMessage=true; caps.mMessageId=sIntrusiveMessage; }
    mSpinAction = cNPCSpinDefault; mTrampledAction = cNPCTrampledDefault; mPointingAction = cNPCPointingDefault; mReactionAction = cNPCReactionDefault;
    initialize(iter,caps,"LuigiNPC",nullptr,nullptr);
    MR::registerEventFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&LuigiTalkNpc::eventFunc));
    if(MR::tryRegisterDemoCast(this,iter) && MR::isDemoCast(this,sIntrusiveDemo)) {
        DemoFunction::registerDemoTalkMessageCtrlDirect(this,mMsgCtrl,sIntrusiveDemo);
        MR::registerDemoActionFunctorDirect(this,MR::Functor_Inline(this,&LuigiTalkNpc::pushTurnPlayer),sIntrusiveDemo,nullptr);
    }
    if(MR::isStageMarioFaceShip() && MR::isMarioFaceShipNpc(this,-1)) {
        TVec3f offset=MR::getMessageBalloonFollowOffset(mMsgCtrl);
        TalkMessageCtrl* ctrl=MR::createTalkCtrlDirectOnRootNodeAutomatic(this,iter,"LuigiIntrusivelyStart",offset,nullptr);
        MR::registerDemoActionFunctorDirect(this,MR::Functor_Inline(this,&LuigiTalkNpc::dummyDemoFunc),sAttentionDemo,nullptr);
        DemoFunction::registerDemoTalkMessageCtrlDirect(this,ctrl,sAttentionDemo);
    }
    if(MR::isObjectName(iter,"LuigiIntrusively")) {
        bool useCamera=false;
        MR::getJMapInfoArg7NoInit(iter,&useCamera);
        if(useCamera) MR::initTalkCamera(mMsgCtrl,iter);
        MR::registerBranchFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&LuigiTalkNpc::branchFuncIntrusively));
        if(!MR::isProductGameScene() || GameSequenceFunction::isLuigiIntrusivelyAppear()) { makeActorAppeared(); GameDataFunction::onLuigiIntrusivelyAppear(); }
        else makeActorDead();
    } else {
        MR::registerBranchFunc(getMsgCtrl(), TalkMessageFunc_Inline(this,&LuigiTalkNpc::branchFuncTalkNpc));
        MR::getJMapInfoArg0NoInit(iter,&mHideAfterIntrusiveUnlock);
        makeActorDead();
    }
}
void LuigiTalkNpc::makeActorAppeared() {
    if(!mHideAfterIntrusiveUnlock || !GameDataFunction::isOnLuigiIntrusivelyStart()) LiveActor::makeActorAppeared();
}
void LuigiTalkNpc::control() {
    NPCActor::control();
    if(isPointingSe()) MR::startActionSound(this,"SvLuigiPoint",-1,-1,-1);
}
void LuigiTalkNpc::exeTurnPlayer() {
    if(MR::isFirstStep(this)) {
        TVec3f front(0.0f);
        MR::calcVecToPlayerH(&front,this,nullptr);
        MR::setNPCActorPose(this,front,-mGravity,mPosition);
    }
    if(!MR::isDemoActive()) popNerve();
}
void LuigiTalkNpc::exeInformation() {
    if(MR::isFirstStep(this)) MR::appearInformationMessageCenter("InformationLuigiIntrusively",true);
    if(MR::isDeadInformationMessage()) popNerve();
}
void LuigiTalkNpc::dummyDemoFunc() {}
bool LuigiTalkNpc::eventFunc(u32 id) {
    switch(id) {
    case 0: {
        TPos3f joint;
        PSMTXCopy(MR::getJointMtx(this,"Head"),joint.toMtxPtr());
        TVec3f position;
        TVec3f offset(0.0f,-20.0f,0.0f);
        joint.mult(offset,position);
        GameSequenceFunction::prepareStageResetLuigiIntrusivelyLuigi(position);
        return true;
    }
    case 1: GameSequenceFunction::prepareStageResetLuigiIntrusivelyMario(mPosition); return true;
    case 2: MR::startAction(this,"Jump"); MR::stopStageBGM(30); return true;
    case 3: return MR::getBckFrame(this)>45.0f;
    case 10:
        if(!isNerve(&NrvLuigiTalkNpc::LuigiTalkNpcNrvInformation::sInstance)) { pushNerve(&NrvLuigiTalkNpc::LuigiTalkNpcNrvInformation::sInstance); return true; }
        return true;
    case 11: return !isNerve(&NrvLuigiTalkNpc::LuigiTalkNpcNrvInformation::sInstance);
    default: return false;
    }
}
bool LuigiTalkNpc::branchFuncTalkNpc(u32 id) { return MR::branchFuncGameProgress(id); }
bool LuigiTalkNpc::branchFuncIntrusively(u32 id) {
    if(id==0) return !isNerve(&NrvLuigiTalkNpc::LuigiTalkNpcNrvInformation::sInstance);
    return true;
}
void LuigiTalkNpc::pushTurnPlayer() { pushNerve(&NrvLuigiTalkNpc::LuigiTalkNpcNrvTurnPlayer::sInstance); }
