#include "Game/Enemy/LavaBubble.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util/SupportPlayUtil.hpp"

#define GET_LAVABUBBLE_NERVE(name) (&NrvLavaBubble::name::sInstance)

namespace NrvLavaBubble {
    NEW_NERVE(LavaBubbleNrvWait, LavaBubble, Wait);
    NEW_NERVE(LavaBubbleNrvLaunch, LavaBubble, Launch);
};  // namespace NrvLavaBubble

LavaBubble::LavaBubble(const char* pName) : LiveActor(pName) {
    mHomePos.x = 0.0f;
    mHomePos.y = 0.0f;
    mHomePos.z = 0.0f;
    mUpVec.x = 0.0f;
    mUpVec.y = 0.0f;
    mUpVec.z = 0.0f;
    mPrevGravity.x = 0.0f;
    mPrevGravity.y = 0.0f;
    mPrevGravity.z = 0.0f;
    mUp.x = 0.0f;
    mUp.y = 0.0f;
    mUp.z = 0.0f;
    mUnk120.x = 0.0f;
    mUnk120.y = 0.0f;
    mUnk120.z = 0.0f;
    mSavedVel.x = 0.0f;
    mSavedVel.y = 0.0f;
    mSavedVel.z = 0.0f;
    mRange = 1000.0f;
    mSpeed = 1.0f;
    mUseSavedGravity = 1;
    mUseFlippedGravity = 0;
    mLaunchDuration = 100;
    mBounceCooldown = 0;
    mLaunchDelay = 90;
    mLaunchCount = 0;
    mBaseMtx.identity();
    mEffectMtx.identity();
}

void LavaBubble::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    initNerve(GET_LAVABUBBLE_NERVE(LavaBubbleNrvWait), 0);
    initPose();

    f32 arg1 = -1.0f;
    f32 arg2 = -1.0f;
    MR::getJMapInfoArg0NoInit(rIter, &mRange);
    MR::getJMapInfoArg1NoInit(rIter, &arg1);
    MR::getJMapInfoArg2NoInit(rIter, &arg2);
    MR::getJMapInfoArg3NoInit(rIter, &mLaunchDuration);
    MR::getJMapInfoArg4NoInit(rIter, &mLaunchDelay);
    if (-1.0f != arg1) {
        mUseSavedGravity = 0;
    }
    if (-1.0f != arg2) {
        mUseFlippedGravity = 1;
    }
    mEffectMtx.setInline(mBaseMtx);
    MR::setEffectHostMtx(this, "Sign", mEffectMtx);
    MR::setGroupClipping(this, rIter, 16);
    MR::useStageSwitchAwake(this, rIter);
    makeActorAppeared();
}

void LavaBubble::calcAndSetBaseMtx() {
    TPos3f mtx;
    mtx.identity();
    TVec3f front(0.0f);
    MR::calcFrontVec(&front, this);
    MR::makeMtxUpFrontPos(&mtx, mUp, front, mPosition);
    MR::setBaseTRMtx(this, mtx);
}

void LavaBubble::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        if (MR::sendMsgEnemyAttackFire(pReceiver, pSender)) {
            return;
        }
    }
}

void LavaBubble::control() {
    TVec3f toPlayer(*MR::getPlayerPos());
    toPlayer.sub(toPlayer, mHomePos);
    if (toPlayer.length() < 2000.0f) {
        TVec3f dir(0.0f);
        if (mUseFlippedGravity != 0) {
            dir.set(-mPrevGravity);
        } else {
            dir.set(mUpVec);
        }
        dir *= 200.0f;
        TVec3f start(mHomePos);
        start += dir;
        dir = dir * -1.0f;
        Triangle tri;
        TVec3f hit;
        if (MR::getFirstPolyOnLineToMapExceptActor(&hit, &tri, start, dir, this)) {
            mEffectMtx[0][3] = hit.x;
            mEffectMtx[1][3] = hit.y;
            mEffectMtx[2][3] = hit.z;
            MR::emitEffect(this, "Sign");
        }
    } else if (MR::isEffectValid(this, "Sign")) {
        MR::forceDeleteEffect(this, "Sign");
    }
    updateUpVecFromVelocity();
}

void LavaBubble::startClipped() {
    LiveActor::startClipped();
    if (MR::isEffectValid(this, "Sign")) {
        MR::forceDeleteEffect(this, "Sign");
    }
}

void LavaBubble::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::validateClipping(this);
    }
    if (MR::isGreaterStep(this, mLaunchDelay)) {
        resetPose();
        setNerve(GET_LAVABUBBLE_NERVE(LavaBubbleNrvLaunch));
        launchBubble();
    }
}

// NONMATCHING: launch-duration and launch-count loads are scheduled in reverse order.
void LavaBubble::exeLaunch() {
    if (MR::isFirstStep(this)) {
        MR::tryDeleteEffect(this, "Sign");
        MR::startActionSound(this, "Launch", -1, -1, -1);
        mSavedVel.set(mVelocity);
        mLaunchCount = 0;
    }
    if (MR::attachSupportTicoToTarget(this)) {
        MR::zeroVelocity(this);
        return;
    }
    mVelocity.set(mSavedVel);
    f32 speed = mSpeed;
    mVelocity.add(getFallDir() * speed);
    mSavedVel.set(mVelocity);
    tryBounceOnGround();
    if (mLaunchCount == mLaunchDuration - 14) {
        MR::startActionSound(this, "BackInMagma", -1, -1, -1);
    }
    s32 duration = mLaunchDuration;
    s32 count = mLaunchCount;
    if (duration < count) {
        setNerve(GET_LAVABUBBLE_NERVE(LavaBubbleNrvWait));
        MR::zeroVelocity(this);
        return;
    }
    mLaunchCount = count + 1;
}

void LavaBubble::initPose() {
    mHomePos.set(mPosition);
    MR::calcUpVec(&mUpVec, this);
    MR::makeMtxTR(mBaseMtx, this);
    mUp.set(mUpVec);
    MR::calcGravity(this);
    mPrevGravity.set(mGravity);
}

void LavaBubble::resetPose() {
    mPosition.set(mHomePos);
    mUp.set(mUpVec);
    MR::setBaseTRMtx(this, mBaseMtx);
    mUp.dot(mGravity);
    MR::zeroVelocity(this);
    mBounceCooldown = 0;
}

void LavaBubble::updateUpVecFromVelocity() {
    if (isNerve(GET_LAVABUBBLE_NERVE(LavaBubbleNrvWait))) {
        return;
    }
    f32 mag = mSavedVel.length();
    TVec3f vel(mSavedVel);
    if (MR::isNearZero(mag, 6.0f * mSpeed)) {
        if (mUp.dot(mGravity) > 0.2f) {
            MR::vecBlend(&mUp, mUp, mGravity, 0.1f);
            MR::normalize(&mUp, mUp);
            return;
        }
        TVec3f front(0.0f);
        MR::calcFrontVec(&front, this);
        MR::vecBlend(&mUp, mUp, front, 0.1f);
        MR::normalize(&mUp, mUp);
        return;
    }
    MR::normalize(&vel);
    MR::vecBlend(&mUp, mUp, vel, 0.2f);
    MR::normalize(&mUp, mUp);
}

void LavaBubble::launchBubble() {
    s32 step = 40;
    f32 size = step;
    mSpeed = 2.0f * (mRange / (size * size));
    f32 launchSpeed = mSpeed * size;
    TVec3f vel;
    TVec3f dir;
    if (mUseFlippedGravity != 0) {
        TVec3f flipped = -mPrevGravity;
        dir.set(flipped);
    } else {
        dir.set(mUp);
    }
    MR::calcDirectionWithLength(&vel, dir, launchSpeed);
    mVelocity.set(vel);
    MR::invalidateClipping(this);
}

bool LavaBubble::tryBounceOnGround() {
    mBounceCooldown -= 1;
    if (mBounceCooldown > 0) {
        return false;
    }
    TVec3f up;
    MR::calcUpVec(&up, this);
    up *= 50.0f;
    TVec3f dir;
    TVec3f down(up);
    down *= -1.0f;
    dir = down;
    if (MR::isExistMapCollision(mPosition, up)) {
        MR::emitEffect(this, "Land");
        mBounceCooldown = 10;
        return true;
    }
    TVec3f probe(mPosition);
    probe.add(up);
    if (MR::isExistMapCollision(probe, dir)) {
        MR::emitEffect(this, "Appear");
        mBounceCooldown = 10;
        return true;
    }
    return false;
}

TVec3f LavaBubble::getFallDir() const {
    if (mUseSavedGravity != 0) {
        return mPrevGravity;
    }
    return mGravity;
}

LavaBubble::~LavaBubble() {}
