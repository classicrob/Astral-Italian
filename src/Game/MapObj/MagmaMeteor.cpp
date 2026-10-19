#pragma auto_inline off
#include "Game/MapObj/MagmaMeteor.hpp"
#include "Game/MapObj/MagmaMeteorParts.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
namespace {
    const char* sMeteorSensor = "Meteor";
    const char* sShadowName = "Body";
    const char* sBodySensor = "Body";
}
namespace { extern const f32 sFireRadiusMin; }
namespace NrvMagmaMeteor {
    NEW_NERVE(MagmaMeteorNrvFirstWait, MagmaMeteor, FirstWait);
    NEW_NERVE(MagmaMeteorNrvFall, MagmaMeteor, Fall);
    NEW_NERVE(MagmaMeteorNrvSpread, MagmaMeteor, Spread);
    NEW_NERVE(MagmaMeteorNrvWait, MagmaMeteor, Wait);
    NEW_NERVE(MagmaMeteorNrvShrink, MagmaMeteor, Shrink);
    NEW_NERVE(MagmaMeteorNrvInterval, MagmaMeteor, Interval);
}
MagmaMeteor::MagmaMeteor(const char* name)
    : LiveActor(name), mFireActive(false), mFireRadius(sFireRadiusMin), mMeteorVelocity(0.0f, 0.0f, 0.0f), mUp(0.0f, 0.0f, 0.0f),
      mFirstWait(0), mInterval(0), mHeight(2000.0f), mFallTime(180), mMeteor(nullptr) {}
MagmaMeteor::~MagmaMeteor() {}
void MagmaMeteor::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    initNerve(&NrvMagmaMeteor::MagmaMeteorNrvFirstWait::sInstance);
    MR::calcGravity(this);
    MR::calcUpVec(&mUp, this);
    MR::getJMapInfoArg0NoInit(iter, &mFirstWait);
    MR::getJMapInfoArg1NoInit(iter, &mInterval);
    MR::getJMapInfoArg2NoInit(iter, &mHeight);
    MR::getJMapInfoArg3NoInit(iter, &mFallTime);
    mMeteor = new MagmaMeteorParts(this, "マグマメテオ隕石部分");
    mMeteor->initWithoutIter();
    MR::setSensorFollowPos(this, sMeteorSensor, &mMeteor->mPosition);
    MR::setShadowDropPositionPtr(this, sShadowName, &mMeteor->mPosition);
    MR::startBrkAndSetFrameAndStop(this, "Spread", 0.0f);
    MR::startBvaAndSetFrameAndStop(this, "Spread", 0.0f);
    MR::startBtkAndSetFrameAndStop(this, "Spread", 0.0f);
    makeActorAppeared();
}
void MagmaMeteor::exeFirstWait() {
    if (MR::isFirstStep(this)) {
        resetMeteor();
        MR::tryEmitEffect(mMeteor, "MagmaMeteorBall");
    }
    if (MR::isGreaterStep(this, mFirstWait)) setNerve(&NrvMagmaMeteor::MagmaMeteorNrvFall::sInstance);
}
void MagmaMeteor::exeFall() {
    updateMeteor();
    if (MR::isStep(this, 10)) MR::startActionSound(mMeteor, "Fall");
    if (MR::isStep(this, mFallTime - 20)) setNerve(&NrvMagmaMeteor::MagmaMeteorNrvSpread::sInstance);
    if (MR::isGreaterStep(this, 10)) MR::startActionSound(mMeteor, "FallLv", 2000 - (s32)MR::getMicDistance(mMeteor));
}
void MagmaMeteor::exeSpread() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Spread");
        mFireActive = true;
    }
    updateMeteor();
    if (MR::isGreaterStep(this, 20)) expandFire();
    if (MR::isActionEnd(this)) setNerve(&NrvMagmaMeteor::MagmaMeteorNrvWait::sInstance);
}
void MagmaMeteor::exeWait() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Wait");
    expandFire();
    if (MR::isGreaterStep(this, 240)) setNerve(&NrvMagmaMeteor::MagmaMeteorNrvShrink::sInstance);
}
void MagmaMeteor::exeShrink() {
    if (MR::isFirstStep(this)) MR::startAction(this, "Shrink");
    shrinkFire();
    if (MR::isActionEnd(this)) setNerve(&NrvMagmaMeteor::MagmaMeteorNrvInterval::sInstance);
}
void MagmaMeteor::exeInterval() {
    if (MR::isFirstStep(this)) resetMeteor();
    if (MR::isGreaterStep(this, mInterval)) setNerve(&NrvMagmaMeteor::MagmaMeteorNrvFall::sInstance);
}
void MagmaMeteor::attackSensor(HitSensor* sender, HitSensor* receiver) {
    if (MR::isSensorPlayer(receiver)) {
        if (MR::isEqualSensor(sender, this, sMeteorSensor)) {
            MR::sendMsgEnemyAttackFire(receiver, sender);
            return;
        }
        if (MR::isEqualSensor(sender, this, sBodySensor) && isPlayerInFire(sender)) MR::sendMsgFireDance(receiver, sender);
    }
}
// NONMATCHING: double-conversion constant address uses r5 instead of r4.
void MagmaMeteor::resetMeteor() {
    mMeteor->setOffset(mUp * mHeight);
    f32 speed = mHeight / mFallTime;
    mMeteorVelocity = -mUp * speed;
    mFireActive = false;
    mFireRadius = sFireRadiusMin;
}
void MagmaMeteor::updateMeteor() {
    TVec3f offset(mMeteor->getOffset());
    if (offset.dot(mUp) > -400.0f) {
        offset += mMeteorVelocity;
        mMeteor->setOffset(offset);
    }
}
bool MagmaMeteor::isPlayerInFire(HitSensor* sensor) const {
    if (!mFireActive) return false;
    TVec3f up;
    MR::calcUpVec(&up, this);
    f32 radius, height;
    MR::func_8003DEF0(&radius, &height, MR::getSensorPos(sensor), MR::getPlayerPos(), &up);
    if (height > 50.0f) return false;
    return radius < mFireRadius;
}
void MagmaMeteor::expandFire() {
    s32 duration = 82;
    mFireRadius += (420.0f - sFireRadiusMin) / duration;
    mFireRadius = MR::clamp(mFireRadius, sFireRadiusMin, 420.0f);
}
void MagmaMeteor::shrinkFire() {
    s32 duration = 62;
    mFireRadius -= 420.0f / duration;
    mFireRadius = MR::clamp(mFireRadius, 0.0f, 420.0f);
}

namespace {
    const f32 sFireRadiusMin = 0.0f;
}
