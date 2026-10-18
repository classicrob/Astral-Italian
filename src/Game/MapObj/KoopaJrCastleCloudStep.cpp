#pragma auto_inline off
#include "Game/MapObj/KoopaJrCastleCloudStep.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SpringValue.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

extern "C" {
void fn_80068E40(SpringValue*, f32);
s32 fn_800D9830(const TVec3f*, TVec3f*, f32*);
}

class DemoObj : public LiveActor {};

namespace NrvKoopaJrCastleCloudStep {
    NEW_NERVE(KoopaJrCastleCloudStepNrvWait, KoopaJrCastleCloudStep, Wait);
    NEW_NERVE(KoopaJrCastleCloudStepNrvBounce, KoopaJrCastleCloudStep, Bounce);
    NEW_NERVE(KoopaJrCastleCloudStepNrvBorn, KoopaJrCastleCloudStep, Born);
    NEW_NERVE(KoopaJrCastleCloudStepNrvDie, KoopaJrCastleCloudStep, Die);
};  // namespace NrvKoopaJrCastleCloudStep

KoopaJrCastleCloudStep::KoopaJrCastleCloudStep(const char* pName) : LiveActor(pName) {
    mDemoObj = nullptr;
    mSpring = nullptr;
    mHomePos.zero();
    mBounceTimer = 0;
    mElecTimer = 0;
    mBrkFrame = 0.0f;
    mUseShadow = false;
    mSpring = new SpringValue(0.0f, 0.0f, 0.03f, 0.92f, 0.0f);
}

void KoopaJrCastleCloudStep::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    mHomePos = mPosition;
    mDemoObj = MR::createDemoObj("アルファモデル", "KoopaJrCastleCloudStepAlpha", getBaseMtx());
    MR::startBrk(mDemoObj, "Appear");
    MR::setBrkFrameEndAndStop(mDemoObj);
    mBrkFrame = MR::getBrkFrameMax(mDemoObj);
    bool useShadow = true;
    MR::getJMapInfoArg0NoInit(rIter, &useShadow);
    mUseShadow = useShadow;
    if (!mUseShadow) {
        MR::invalidateShadowAll(this);
    }
    initNerve(GET_NERVE(KoopaJrCastleCloudStep, KoopaJrCastleCloudStepNrvWait), 0);
    updateBrkByCamera();
    makeActorAppeared();
}

void KoopaJrCastleCloudStep::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    mDemoObj->makeActorAppeared();
}

void KoopaJrCastleCloudStep::makeActorDead() {
    LiveActor::makeActorDead();
    mDemoObj->makeActorDead();
}

void KoopaJrCastleCloudStep::control() {
    if (mBounceTimer > 0) {
        mBounceTimer--;
    }
    if (mElecTimer > 0) {
        mElecTimer--;
    }
    updateBrkByCamera();
    mSpring->update();
    fn_80068E40(mSpring, 60.0f);
}

// NONMATCHING: target copies getBaseMtx() with unrolled psq pairs, ours is a bdnz struct-copy loop (~81%).
void KoopaJrCastleCloudStep::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    TPos3f mtx;
    mtx = *reinterpret_cast< const TPos3f* >(getBaseMtx());
    TVec3f offset = mGravity * mSpring->mSpringValue;
    MR::addTransMtx(mtx.mMtx, offset);
    MR::setBaseTRMtx(this, mtx);

    f32 t = 1.0f - 0.25f * (mSpring->mSpringValue / 60.0f);
    f32 s = 0.05f;
    if (t >= 0.05f) {
        s = 2.0f;
        if (t <= 2.0f) {
            s = t;
        }
    }

    TVec3f scale = mScale;
    f32 r = MR::sqrt(1.0f / s);
    scale.mul(TVec3f(r, s, r));
    MR::setBaseScale(this, scale);
    if (mDemoObj != nullptr) {
        MR::syncJointAnimation(mDemoObj, this);
        MR::setBaseScale(mDemoObj, scale);
    }
}

bool KoopaJrCastleCloudStep::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDropFloor(msg)) {
        tryDie();
        return true;
    }
    return false;
}

bool KoopaJrCastleCloudStep::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgEnemyAttackElectric(msg)) {
        mElecTimer = 10;
        tryDie();
        return true;
    }
    return false;
}

bool KoopaJrCastleCloudStep::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0xF4) {
        bool isHome = false;
        if (JGeometry::TUtil< f32 >::epsilonEquals(mPosition.x, mHomePos.x, MR::epsilon()) &&
            JGeometry::TUtil< f32 >::epsilonEquals(mPosition.y, mHomePos.y, MR::epsilon()) &&
            JGeometry::TUtil< f32 >::epsilonEquals(mPosition.z, mHomePos.z, MR::epsilon())) {
            isHome = true;
        }
        if (isHome) {
            mElecTimer = 10;
        }
        tryDie();
        return true;
    }

    if (MR::isPlayerElementModeBee() || MR::isPlayerElementModeCloud()) {
        if (MR::isMsgFloorTouch(msg)) {
            if (mBounceTimer == 0) {
                mSpring->mVelocity += 8.0f;
            }
            mBounceTimer = 10;
        }
        return false;
    }

    if (MR::isMsgFloorTouch(msg) || MR::isMsgWallTouch(msg) || MR::isMsgCeilTouch(msg)) {
        tryDie();
    }
    return false;
}

bool KoopaJrCastleCloudStep::tryDie() {
    if (isNerve(GET_NERVE(KoopaJrCastleCloudStep, KoopaJrCastleCloudStepNrvWait)) ||
        isNerve(GET_NERVE(KoopaJrCastleCloudStep, KoopaJrCastleCloudStepNrvBounce))) {
        setNerve(GET_NERVE(KoopaJrCastleCloudStep, KoopaJrCastleCloudStepNrvDie));
        return true;
    }
    return false;
}

bool KoopaJrCastleCloudStep::updateWind() {
    bool ret = false;
    TVec3f dir(0.0f);
    f32 power = -1.0f;
    if (fn_800D9830(&mPosition, &dir, &power)) {
        TVec3f* pGravity = &mGravity;
        JMAVECScaleAdd(pGravity, &dir, &dir, -pGravity->dot(dir));
        if (!MR::isNearZero(dir, 0.001f)) {
            ret = true;
        }
        TVec3f vel = dir * 0.2f;
        MR::addVelocity(this, vel);
    }
    MR::attenuateVelocity(this, 0.995f);
    MR::killVelocityVertical(this);
    return ret;
}

void KoopaJrCastleCloudStep::updateBrkByCamera() {
    TVec3f camPos = MR::getCamPos();
    TVec3f rel(mPosition);
    rel -= camPos;
    TVec3f camZdir = MR::getCamZdir();
    f32 dot = camZdir.dot(rel);

    bool dec = false;
    bool inc = false;
    f32 maxBrk = MR::getBrkFrameMax(mDemoObj);
    if (mBrkFrame < maxBrk) {
        if (dot < 800.0f) {
            dec = true;
        } else if (dot > 1000.0f) {
            inc = true;
        }
    } else {
        if (dot > 1000.0f) {
            inc = true;
        } else if (dot < 800.0f) {
            dec = true;
        }
    }

    if (dec) {
        mBrkFrame -= 1.0f;
        if (mBrkFrame <= 0.0f) {
            mBrkFrame = 0.0f;
        }
        if (!MR::isHiddenModel(this) && !MR::isDead(mDemoObj)) {
            MR::hideModelAndOnCalcAnim(this);
        }
        if (MR::isDead(mDemoObj)) {
            mDemoObj->appear();
        }
    }

    if (inc) {
        mBrkFrame += 1.0f;
        f32 maxBrkInc = MR::getBrkFrameMax(mDemoObj);
        if (mBrkFrame >= maxBrkInc) {
            mBrkFrame = MR::getBrkFrameMax(mDemoObj);
            if (!MR::isDead(mDemoObj) && !MR::isHiddenModel(this)) {
                mDemoObj->kill();
            }
            if (MR::isHiddenModel(this)) {
                MR::showModel(this);
            }
        }
    }

    MR::setBrkFrameAndStop(mDemoObj, mBrkFrame);
}

void KoopaJrCastleCloudStep::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateCollisionParts(this);
        MR::tryStartActionNotPlaying(this, "KoopaJrCastleCloudStep");
    }
    if (updateWind()) {
        setNerve(GET_NERVE(KoopaJrCastleCloudStep, KoopaJrCastleCloudStepNrvBounce));
    }
}

void KoopaJrCastleCloudStep::exeBounce() {
    if (!updateWind()) {
        tryDie();
    }
    if (mBounceTimer > 0 && getNerveStep() % 10 != 0) {
        MR::tryRumblePadWeakAndVeryWeak(this, 0);
    }
}

void KoopaJrCastleCloudStep::exeBorn() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Born");
        MR::validateCollisionParts(this);
        if (mUseShadow) {
            MR::validateShadowAll(this);
        }
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(KoopaJrCastleCloudStep, KoopaJrCastleCloudStepNrvWait));
    }
}

void KoopaJrCastleCloudStep::exeDie() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Die");
        MR::invalidateCollisionParts(this);
        MR::zeroVelocity(this);
    }
    if (MR::isStep(this, 20) && mUseShadow) {
        MR::invalidateShadowAll(this);
    }
    if (!updateWind()) {
        MR::zeroVelocity(this);
        if (MR::isGreaterStep(this, 60) && mElecTimer == 0) {
            mPosition = mHomePos;
            setNerve(GET_NERVE(KoopaJrCastleCloudStep, KoopaJrCastleCloudStepNrvBorn));
        }
    }
}

KoopaJrCastleCloudStep::~KoopaJrCastleCloudStep() {}
