#pragma auto_inline off
#include "Game/NPC/Monte.hpp"
#include "Game/NPC/MonteThrow.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JointRumbler.hpp"

namespace NrvMonte { NEW_NERVE(Throw,Monte,Throw); NEW_NERVE(Wait,Monte,Wait); }
Monte::Monte(const char* name) : NPCActor(name),mType(-1),mColor(-1),mThrow(0),mRumbler(0) {}


// NONMATCHING: register allocation and string/nerve address scheduling differ.
void Monte::init(const JMapInfoIter& iter) {
    NPCActorCaps caps("Monte");
    caps.setDefault();
    initialize(iter,caps,"Monte",0,0);
    mWaitNerve=&NrvMonte::Wait::sInstance;
    setNerve(mWaitNerve);
    MR::getJMapInfoArg0NoInit(iter,&mColor);
    MR::startBrkAndSetFrameAndStop(this,"ColorChange",mColor);
    MR::getJMapInfoArg1NoInit(iter,&mType);
    if(mType==-1) {
        mParam._14="Wait";
        mParam._18="Turn";
        mParam._1C="Talk";
        mParam._20="Turn";
    } else if(mType==0) {
        mParam._14="WaitThrow";
        mParam._1C="WaitThrow";
        mParam._18=0;
        mParam._20=0;
        mParam._0=0;
        mParam._1=0;
        mThrow=new MonteThrow(this);
        mThrow->init(iter);
    }
    mSpinAction=cNPCSpinDefault;
    mTrampledAction=cNPCTrampledDefault;
    mPointingAction=cNPCPointingDefault;
    mReactionAction=cNPCReactionDefault;
    mSpinAttackRange=450.0f;
    MR::onCalcGravity(this);
    mRumbler=new JointRumbler(this,"Center",3.0f,0.1f,30,true,0);
    makeActorAppeared();
}


void Monte::initAfterPlacement() { MR::initEffectAfterPlacement(this,0); }


void Monte::control() { if(!tryStartThrow()) NPCActor::control(); mRumbler->update(); }


void Monte::attackSensor(HitSensor* sender,HitSensor* receiver) {
    if(MR::isSensorPlayerOrRide(receiver)) MR::sendMsgPush(receiver,sender);
    if(MR::isSensorEnemy(receiver) && MR::sendMsgPush(receiver,sender)) mRumbler->start();
}


bool Monte::receiveMsgPlayerAttack(u32 msg,HitSensor* sender,HitSensor* receiver) {
    if(mType==0 && (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg))) return false;
    if(MR::isMsgYoshiProjectile(msg)) { mReacting=1; return true; }
    return NPCActor::receiveMsgPlayerAttack(msg,sender,receiver);
}


bool Monte::receiveOtherMsg(u32 msg,HitSensor* sender,HitSensor* receiver) {
    if(mThrow) return mThrow->receiveOtherMsg(msg,sender,receiver);
    return NPCActor::receiveOtherMsg(msg,sender,receiver);
}


bool Monte::tryStartThrow() {
    if(mType!=0) return false;
    if(isNerve(&NrvMonte::Throw::sInstance)) return false;
    if(mPointed) return false;
    if(!mThrow->isPreparingThrow()) return false;
    if(isNerve(mReactionNerve)) popNerve();
    setNerve(&NrvMonte::Throw::sInstance);
    return true;
}


void Monte::exeWait() {
    if(mType==-1) { NPCActor::exeWait(); return; }
    if(mType==0) {
        if(MR::isFirstStep(this) && !MR::isSensorValid(getSensor("Body"))) MR::validateHitSensor(this,"Body");
        if(!MR::tryStartReactionAndPushNerve(this,mReactionNerve) && !MR::tryStartActionNotPlaying(this,mParam._14)) {
            if(getMsgCtrl()) MR::tryTalkNearPlayer(getMsgCtrl());
        }
    }
}


void Monte::exeThrow() {
    if(MR::isFirstStep(this)) MR::startAction(this,"Throw");
    if(MR::isStep(this,8)) { mThrow->startThrow(); MR::invalidateHitSensor(this,"Body"); }
    if(MR::isActionEnd(this)) { MR::validateHitSensor(this,"Body"); setNerve(mWaitNerve); }
}


Monte::~Monte() {}
