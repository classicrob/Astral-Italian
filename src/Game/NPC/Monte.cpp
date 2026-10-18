#pragma auto_inline off
#include "Game/NPC/Monte.hpp"
#include "Game/NPC/MonteThrow.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JointRumbler.hpp"
extern "C" {
extern char lbl_806BB090[],lbl_806BB0D0[],lbl_806BB0D8[];
extern char lbl_806B7AEC[],lbl_806B7AE0[],lbl_806B7AD4[],lbl_806B7AC8[];
}
namespace NrvMonte { NEW_NERVE(Throw,Monte,Throw); NEW_NERVE(Wait,Monte,Wait); }
#define RAW(T,o) (*(T*)((u8*)this+(o)))
Monte::Monte(const char* name) : NPCActor(name),mType(-1),mColor(-1),mThrow(0),mRumbler(0) {}
Monte::~Monte() {}
// NONMATCHING: register allocation and string/nerve address scheduling differ.
void Monte::init(const JMapInfoIter& iter) {
    const char* strings=lbl_806BB090;
    NPCActorCaps caps(strings);
    caps.setDefault();
    initialize(iter,caps,strings,0,0);
    RAW(Nerve*,0x154)=&NrvMonte::Wait::sInstance;
    setNerve(RAW(Nerve*,0x154));
    MR::getJMapInfoArg0NoInit(iter,&mColor);
    MR::startBrkAndSetFrameAndStop(this,strings+8,mColor);
    MR::getJMapInfoArg1NoInit(iter,&mType);
    if(mType==-1) {
        RAW(const char*,0x100)=strings+0x1C;
        RAW(const char*,0x104)=strings+0x14;
        RAW(const char*,0x108)=strings+0x24;
        RAW(const char*,0x10C)=strings+0x14;
    } else if(mType==0) {
        RAW(const char*,0x100)=strings+0x2C;
        RAW(const char*,0x108)=strings+0x2C;
        RAW(const char*,0x104)=0;
        RAW(const char*,0x10C)=0;
        RAW(u8,0xEC)=0;
        RAW(u8,0xED)=0;
        mThrow=new MonteThrow(this);
        mThrow->init(iter);
    }
    RAW(const char*,0x134)=lbl_806B7AEC;
    RAW(const char*,0x138)=lbl_806B7AE0;
    RAW(const char*,0x13C)=lbl_806B7AD4;
    RAW(const char*,0x140)=lbl_806B7AC8;
    RAW(f32,0x130)=450.0f;
    MR::onCalcGravity(this);
    mRumbler=new JointRumbler(this,strings+0x38,3.0f,0.1f,30,true,0);
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
    if(MR::isMsgYoshiProjectile(msg)) { RAW(u8,0xE9)=1; return true; }
    return NPCActor::receiveMsgPlayerAttack(msg,sender,receiver);
}
bool Monte::receiveOtherMsg(u32 msg,HitSensor* sender,HitSensor* receiver) {
    if(mThrow) return mThrow->receiveOtherMsg(msg,sender,receiver);
    return NPCActor::receiveOtherMsg(msg,sender,receiver);
}
bool Monte::tryStartThrow() {
    if(mType!=0) return false;
    if(isNerve(&NrvMonte::Throw::sInstance)) return false;
    if(RAW(u8,0xE8)) return false;
    if(!mThrow->isPreparingThrow()) return false;
    if(isNerve(RAW(Nerve*,0x15C))) popNerve();
    setNerve(&NrvMonte::Throw::sInstance);
    return true;
}
void Monte::exeWait() {
    if(mType==-1) { NPCActor::exeWait(); return; }
    if(mType==0) {
        if(MR::isFirstStep(this) && !MR::isSensorValid(getSensor(lbl_806BB0D0))) MR::validateHitSensor(this,lbl_806BB0D0);
        if(!MR::tryStartReactionAndPushNerve(this,RAW(Nerve*,0x15C)) && !MR::tryStartActionNotPlaying(this,RAW(const char*,0x100))) {
            if(getMsgCtrl()) MR::tryTalkNearPlayer(getMsgCtrl());
        }
    }
}
void Monte::exeThrow() {
    if(MR::isFirstStep(this)) MR::startAction(this,lbl_806BB0D8);
    if(MR::isStep(this,8)) { mThrow->startThrow(); MR::invalidateHitSensor(this,lbl_806BB0D0); }
    if(MR::isActionEnd(this)) { MR::validateHitSensor(this,lbl_806BB0D0); setNerve(RAW(Nerve*,0x154)); }
}
