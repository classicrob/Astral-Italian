#include "Game/Player/InfernoMarioStateBase.hpp"
#include "Game/Player/InfernoMario.hpp"
#include "Game/Player/InfernoMarioAppearance.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorStateUtil.hpp"
namespace NrvInfernoMarioComet {
    NEW_NERVE(InfernoMarioCometNrvAppearance, InfernoMarioComet, Appearance);
    NEW_NERVE(InfernoMarioCometNrvWait, InfernoMarioComet, Wait);
    NEW_NERVE(InfernoMarioCometNrvFollow, InfernoMarioComet, Follow);
}
InfernoMarioStateBase::InfernoMarioStateBase(const char* name,InfernoMario* host) : ActorStateBase<InfernoMario>(name,host) {}
InfernoMarioStateBase::~InfernoMarioStateBase() {}
void InfernoMarioStateBase::attackSensor(HitSensor*,HitSensor*) {}
bool InfernoMarioStateBase::receiveMsgPlayerAttack(u32,HitSensor*,HitSensor*) { return false; }
bool InfernoMarioStateBase::receiveMsgEnemyAttack(u32,HitSensor*,HitSensor*) { return false; }
bool InfernoMarioStateBase::receiveOtherMsg(u32,HitSensor*,HitSensor*) { return false; }
bool InfernoMarioStateBase::start() { return false; }
bool InfernoMarioStateBase::startFrame(s32) { return false; }
InfernoMarioComet::InfernoMarioComet(InfernoMario* host) : InfernoMarioStateBase("インフェルノマリオ[コメット]",host),mFrame(0) {
    initNerve(&NrvInfernoMarioComet::InfernoMarioCometNrvFollow::sInstance,1);
    MR::initActorState(this,new InfernoMarioAppearance(host),&NrvInfernoMarioComet::InfernoMarioCometNrvAppearance::sInstance,"AppearDemo");
}
InfernoMarioComet::~InfernoMarioComet() {}
void InfernoMarioComet::appear() { mIsDead=false; MR::invalidateClipping(mHost); }
void InfernoMarioComet::attackSensor(HitSensor* sender,HitSensor* receiver) {
    if(isFollowing() && MR::isSensorPlayerOrRide(receiver) && MR::isEqualSensor(sender,mHost,"Attack")) {
        if(MR::sendMsgEnemyAttack(receiver,sender)) { MR::tryHitReactionDead(mHost); kill(); }
    }
}
bool InfernoMarioComet::receiveMsgPlayerAttack(u32,HitSensor*,HitSensor*) { return false; }
bool InfernoMarioComet::receiveOtherMsg(u32,HitSensor*,HitSensor*) { return false; }
bool InfernoMarioComet::start() { setNerve(&NrvInfernoMarioComet::InfernoMarioCometNrvAppearance::sInstance); return true; }
bool InfernoMarioComet::startFrame(s32 frame) {
    mFrame=frame;
    mHost->setFrameDelay(frame);
    mHost->readTransform(mFrame);
    MR::resetPosition(mHost);
    setNerve(&NrvInfernoMarioComet::InfernoMarioCometNrvWait::sInstance);
    return true;
}
bool InfernoMarioComet::isFollowing() const { return isNerve(&NrvInfernoMarioComet::InfernoMarioCometNrvFollow::sInstance); }
void InfernoMarioComet::control() {
    mHost->mCurrentQuat.slerp(mHost->mTargetQuat,0.2f);
    mHost->mCurrentQuat.normalize();
}
void InfernoMarioComet::exeWait() {
    if(MR::isFirstStep(this)) MR::startAction(mHost,"PetitPorterEnd");
    mFrame++;
    if(MR::isGreaterStep(this,60)) setNerve(&NrvInfernoMarioComet::InfernoMarioCometNrvFollow::sInstance);
}
void InfernoMarioComet::exeFollow() {
    MR::startActionSound(mHost,"BmLvGhostMarioAmbient",-1,-1,-1);
    mHost->readPacket(mFrame);
    if(MR::isIntervalStep(this,4)) {
        if(mFrame>mHost->mFrameDelay) mFrame--;
        else if(mFrame<mHost->mFrameDelay) mFrame++;
    }
    if(MR::isPowerStarGetDemoActive() && MR::isNearPlayer(mHost,500.0f)) kill();
}
void InfernoMarioComet::exeAppearance() { if(MR::updateActorState(this)) kill(); }
