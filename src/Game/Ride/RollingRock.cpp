#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#pragma auto_inline off
#include "Game/Ride/RollingRock.hpp"
#include "Game/Camera/CameraDirector.hpp"
#include "Game/Camera/CameraHolder.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/Camera/CameraTargetMtx.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include <JSystem/JMath.hpp>
#include <cmath>


inline void addVec(TVec3f& rDst, const TVec3f& rSrc) {
    JMathInlineVEC::PSVECAdd(&rDst, &rSrc, &rDst);
}
namespace NrvRollingRock {
    NEW_NERVE(RollingRockNrvWait, RollingRock, Wait);
};  // namespace NrvRollingRock

RollingRock::RollingRock(const char* pName)
    : LiveActor(pName), mBlurModel(nullptr), mBreakModel(nullptr), mState(nullptr), mArg7(0), mEffectPos(TVec3f(0.0f)), mFront(0.0f, 0.0f, 1.0f) {
    mQuat.set< f32 >(0.0f, 0.0f, 0.0f, 1.0f);
    mMtx.identity();
    mEffectMtx.identity();
}

void RollingRock::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, MR::isPlayerLuigi() ? "\x52\x6F\x63\x6B\x4C\x75\x69\x67\x69\x52\x6F\x6C\x6C\x69\x6E\x67\x52\x6F\x63\x6B" : "RockMarioRollingRock", false);
    MR::getJMapInfoArg7NoInit(rIter, &mArg7);
    MR::makeQuatFromRotate(&mQuat, this);
    mMtx.setInline(getBaseMtx());
    MR::setSensorFollowPos(this, "Bind", &mEffectPos);
    MR::setSensorFollowPos(this, "Body", &mEffectPos);
    initNerve(&NrvRollingRock::RollingRockNrvWait::sInstance, 1);
    mState = new RollingRockStateParent(this);
    MR::initActorState(this, mState, &NrvRollingRock::RollingRockNrvWait::sInstance, nullptr);
    MR::setAllEffectHostMtx(this, mEffectMtx);
    mBlurModel = createBlurModel(mEffectMtx);
    mBreakModel = createBreakModel(mEffectMtx);
    MR::onCalcGravity(this);
    MR::hideModel(this);
    makeActorDead();
}

void RollingRock::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    MR::offBind(this);
    setNerve(&NrvRollingRock::RollingRockNrvWait::sInstance);
}

void RollingRock::kill() {
    if (!mState->mIsDead) {
        MR::curePlayerElementMode();
        mState->kill();
    }
    LiveActor::kill();
}

void RollingRock::calcAnim() {
    LiveActor::calcAnim();
    TVec3f off(0.0f);
    TVec3f pos(mPosition);
    if (MR::isPlayerInRush() && MR::isPlayerVisible() && mState && !mState->mIsDead) {
        pos.set(*MR::getPlayerPos());
    }
    TVec3f sum(pos);
    addVec(sum, off);
    mEffectPos.set(sum);
}

bool RollingRock::isRollingReady() const {
    bool ret = false;
    if (!mState->mIsDead && mState->isRollingReady()) {
        ret = true;
    }
    return ret;
}

void RollingRock::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
}

void RollingRock::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvRollingRock::RollingRockNrvWait::sInstance)) {
        mState->attackSensor(pSender, pReceiver);
        return;
    }
    if (!MR::isSensorPlayer(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool RollingRock::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvRollingRock::RollingRockNrvWait::sInstance)) {
        return mState->receiveMsgEnemyAttack(msg, pSender, pReceiver);
    }
    return false;
}

bool RollingRock::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvRollingRock::RollingRockNrvWait::sInstance)) {
        return mState->receiveMsgPush(pSender, pReceiver);
    }
    return false;
}

bool RollingRock::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    MR::isMsgRushCancel(msg);
    MR::isMsgRushForceCancel(msg);
    if (isNerve(&NrvRollingRock::RollingRockNrvWait::sInstance)) {
        return mState->receiveOtherMsg(msg, pSender, pReceiver);
    }
    return false;
}

void RollingRock::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
    }
    if (MR::updateActorState(this)) {
        MR::zeroVelocity(this);
        mMtx.getTrans(mPosition);
        MR::offBind(this);
    }
}
