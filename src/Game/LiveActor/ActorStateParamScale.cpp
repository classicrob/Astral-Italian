#include "Game/LiveActor/ActorStateParamScale.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/NerveUtil.hpp"

// MR::clamp / MR::getEaseOutValue are out-of-line in the target; MathUtil.hpp declares clamp inline.
namespace MR {
    f32 clamp(f32 x, f32 min, f32 max);
    f32 getEaseOutValue(f32 x, f32 start, f32 end, f32 max);
}

ActorStateParamScale::ActorStateParamScale(LiveActor* pHost) {
    mIsOnSwitch = false;
    mIsUseSwitch = false;
    mNerveStepRate = 0.0f;
    mPrevNerveStepRate = -1.0f;
    mSpeedOn = 1.0f;
    mSpeedOff = 1.0f;
    mSavedVel.x = 0.0f;
    mSavedVel.y = 0.0f;
    mSavedVel.z = 0.0f;
    mNerve = nullptr;
    mNerveStep = 0;
    mHost = pHost;
    mSwitchActor = pHost;
}

void ActorStateParamScale::init(const JMapInfoIter& rIter) {
    if (MR::useStageSwitchReadParam(mSwitchActor, rIter)) {
        MR::getJMapInfoParamScale(rIter, &mSpeedOn);
        mIsUseSwitch = true;
    }
}

void ActorStateParamScale::init(const ActorStateParamScale* pOther) {
    mIsOnSwitch = pOther->mIsOnSwitch;
    mIsUseSwitch = pOther->mIsUseSwitch;
    mSpeedOn = pOther->mSpeedOn;
    mSpeedOff = pOther->mSpeedOff;
    mSwitchActor = pOther->mSwitchActor;
    f32 speed = pOther->mIsOnSwitch ? pOther->mSpeedOn : pOther->mSpeedOff;
    mPrevNerveStepRate = -speed;
    mNerveStepRate = 0.0f;
}

f32 ActorStateParamScale::getSpeed() {
    if (mIsOnSwitch) {
        return mSpeedOn;
    }
    return mSpeedOff;
}

f32 ActorStateParamScale::getNerveStepRate() {
    return mNerveStepRate;
}

bool ActorStateParamScale::isStep(s32 step) {
    bool result = false;
    if (mPrevNerveStepRate < (f32)step && (f32)step <= mNerveStepRate) {
        result = true;
    }
    return result;
}

bool ActorStateParamScale::isFirstStep() {
    return MR::isFirstStep(mHost);
}

bool ActorStateParamScale::isGreaterStep(s32 step) {
    return mNerveStepRate > (f32)step;
}

bool ActorStateParamScale::isGreaterEqualStep(s32 step) {
    return mNerveStepRate >= (f32)step;
}

bool ActorStateParamScale::isIntervalStep(s32 interval, s32 offset) {
    bool result = false;
    if (offset == (s32)mNerveStepRate % interval && isStep((s32)mNerveStepRate)) {
        result = true;
    }
    return result;
}

void ActorStateParamScale::update() {
    f32 speed;
    if (mIsOnSwitch) {
        speed = mSpeedOn;
    } else {
        speed = mSpeedOff;
    }

    const Nerve* nerve = mHost->mSpine->getCurrentNerve();
    s32 step = mHost->getNerveStep();
    if (isNeedForReset(nerve, step)) {
        mNerveStepRate = 0.0f;
        mPrevNerveStepRate = -speed;
    } else {
        mPrevNerveStepRate = mNerveStepRate;
        mNerveStepRate = mNerveStepRate + speed;
    }
    mNerve = nerve;
    mNerveStep = step;

    if (mIsUseSwitch) {
        if (MR::isExistBck(mHost, nullptr)) {
            if (MR::getBckRate(mHost) > 0.0f) {
                MR::setBckRate(mHost, speed);
            }
        }
        if (MR::isExistBtk(mHost, nullptr)) {
            if (MR::getBtkRate(mHost) > 0.0f) {
                MR::setBtkRate(mHost, speed);
            }
        }
        if (MR::isExistBrk(mHost, nullptr)) {
            if (MR::getBrkRate(mHost) > 0.0f) {
                MR::setBrkRate(mHost, speed);
            }
        }
        if (MR::isExistBtp(mHost, nullptr)) {
            if (MR::getBtpRate(mHost) > 0.0f) {
                MR::setBtpRate(mHost, speed);
            }
        }
        if (MR::isExistBpk(mHost, nullptr)) {
            if (MR::getBpkRate(mHost) > 0.0f) {
                MR::setBpkRate(mHost, speed);
            }
        }
        if (MR::isExistBva(mHost, nullptr)) {
            if (MR::getBvaRate(mHost) > 0.0f) {
                MR::setBvaRate(mHost, speed);
            }
        }
        mIsOnSwitch = MR::isOnSwitchParam(mSwitchActor);
    }
}

void ActorStateParamScale::resetHostVelocity() {
    LiveActor* pHost = mHost;
    pHost->mVelocity.x = mSavedVel.x;
    pHost->mVelocity.y = mSavedVel.y;
    pHost->mVelocity.z = mSavedVel.z;
}

void ActorStateParamScale::calcHostVelocity() {
    mSavedVel = mHost->mVelocity;
    f32 speed = mIsOnSwitch ? mSpeedOn : mSpeedOff;
    mHost->mVelocity.scale(speed);
}

LiveActor* ActorStateParamScale::calcHostVelocity(const TVec3f& rVel) {
    f32 speed = mIsOnSwitch ? mSpeedOn : mSpeedOff;
    TVec3f vel(rVel);
    vel.scale(speed);
    mHost->mVelocity.add(vel);
    return mHost;
}

void ActorStateParamScale::calcHostVelocity(const TVec3f& rVel, const TVec3f& rScale, f32 rate) {
    f32 speed = mIsOnSwitch ? mSpeedOn : mSpeedOff;
    JMAVECScaleAdd(&rVel, &rScale, &mHost->mVelocity, rate * speed);
}

f32 ActorStateParamScale::calcStepRatio(s32 maxStep) {
    if (maxStep <= 0) {
        return 1.0f;
    }
    f32 rate = mNerveStepRate / (f32)maxStep;
    return MR::clamp(rate, 0.0f, 1.0f);
}

void ActorStateParamScale::updateStepRatioEase(s32 maxStep) {
    f32 ratio;
    if (maxStep <= 0) {
        ratio = 1.0f;
    } else {
        f32 rate = mNerveStepRate / (f32)maxStep;
        ratio = MR::clamp(rate, 0.0f, 1.0f);
    }
    MR::getEaseOutValue(ratio, 0.0f, 1.0f, 1.0f);
}

bool ActorStateParamScale::isNeedForReset(const Nerve* pNerve, s32 step) {
    if (MR::isNewNerve(mHost)) {
        return true;
    }
    if (MR::isFirstStep(mHost)) {
        return true;
    }
    if (mNerve != pNerve) {
        return true;
    }
    return mNerveStep >= step;
}
