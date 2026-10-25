#pragma auto_inline off
#include "Game/MapObj/KoopaJrCastleCloudStep.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SpringValue.hpp"
#include "Game/LiveActor/DemoObj.hpp"
#include "Game/Boss/KoopaJrCastle.hpp"
#include "math_types.hpp"
#include <revolution/types.h>


namespace NrvKoopaJrCastleCloudStep {
    NEW_NERVE(KoopaJrCastleCloudStepNrvWait, KoopaJrCastleCloudStep, Wait);
    NEW_NERVE(KoopaJrCastleCloudStepNrvBounce, KoopaJrCastleCloudStep, Bounce);
    NEW_NERVE(KoopaJrCastleCloudStepNrvBorn, KoopaJrCastleCloudStep, Born);
    NEW_NERVE(KoopaJrCastleCloudStepNrvDie, KoopaJrCastleCloudStep, Die);
};  // namespace NrvKoopaJrCastleCloudStep

KoopaJrCastleCloudStep::KoopaJrCastleCloudStep(const char* pName) : LiveActor(pName) {
    mDemoObj = nullptr;
    mSpring = nullptr;
    mHomePos.x = 0.0f;
    mHomePos.y = 0.0f;
    mHomePos.z = 0.0f;
    mBounceTimer = 0;
    mElecTimer = 0;
    mBrkFrame = 0.0f;
    mUseShadow = false;
    mSpring = new SpringValue(0.0f, 0.0f, 0.03f, 0.92f, 0.0f);
}

// NONMATCHING: compiler reuses the shadow argument register instead of reloading its stack byte.
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
    if (!useShadow) {
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
    mSpring->clampValueMax(60.0f);
}

void KoopaJrCastleCloudStep::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    TPos3f mtx;
    mtx.set(*reinterpret_cast< const TPos3f* >(getBaseMtx()));
    MR::addTransMtx(mtx.mMtx, mGravity * mSpring->mSpringValue);
    MR::setBaseTRMtx(this, mtx);

    f32 t = 1.0f - 0.25f * (mSpring->mSpringValue / 60.0f);
    f32 s = MR::clamp(t, 0.05f, 2.0f);

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

// NONMATCHING: spring-add operand registers and common-return code placement differ.
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
                f32 impulse = 8.0f;
                mSpring->mVelocity = mSpring->mVelocity + impulse;
            }
            mBounceTimer = 10;
        }
        return false;
    }

    if (MR::isMsgFloorTouch(msg) || MR::isMsgWallTouch(msg) || MR::isMsgCeilTouch(msg)) {
        tryDie();
        return false;
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
    TVec3f* pGravity;
    bool ret = false;
    TVec3f dir(0.0f);
    f32 power = -1.0f;
    if (KoopaJrCastleWindFunction::calcWindInfo(&mPosition, &dir, &power)) {
        pGravity = &mGravity;
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

// NONMATCHING: camera-vector temporary stack slots and result-flag register allocation differ.
namespace {
    inline TVec3f calcCameraRelativePosition(const TVec3f& position, TVec3f cameraPosition) {
        return position - cameraPosition;
    }
}

void KoopaJrCastleCloudStep::updateBrkByCamera() {
    TVec3f rel(calcCameraRelativePosition(mPosition, MR::getCamPos()));
    f32 dot = MR::getCamZdir().dot(rel);

    bool inc;
    bool dec;
    inc = false;
    dec = false;
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
