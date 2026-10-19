#pragma auto_inline off
#include "Game/MapObj/WaterPressureBullet.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorStateParamScale.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include <JSystem/JMath/JMATrigonometric.hpp>
#include <JSystem/JMath/JMath.hpp>

namespace NrvWaterPressureBullet {
    NEW_NERVE(WaterPressureBulletNrvFly, WaterPressureBullet, Fly);
    NEW_NERVE(WaterPressureBulletNrvSpinKill, WaterPressureBullet, SpinKill);
};  // namespace NrvWaterPressureBullet

WaterPressureBullet::WaterPressureBullet(const char* pName) : LiveActor(pName) {
    mFrontDir.x = 0.0f;
    mFrontDir.y = 0.0f;
    mFrontDir.z = 0.0f;
    mSideDir.x = 0.0f;
    mSideDir.y = 0.0f;
    mSideDir.z = 0.0f;
    mShooterActor = 0;
    mSpeed = 0.0f;
    mHostActor = nullptr;
    mIsNoGravity = false;
    mCanSlideOnGround = false;
    mIsLongLived = false;
    mCameraInfo = nullptr;
}

void WaterPressureBullet::init(const JMapInfoIter& rIter) {
    MR::initActor(this, "WaterBullet", false);
    MR::connectToSceneMapObjStrongLight(this);
    initHitSensor(2);
    MR::addHitSensor(this, "body", 0x1C /* ATYPE_WATER_PRESSURE_BULLET */, 4, 100.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::addHitSensor(this, "binder", 0x64 /* ATYPE_WATER_PRESSURE_BULLET_BIND */, 4, 100.0f, TVec3f(0.0f, 0.0f, 0.0f));
    initBinder(100.0f, 0.0f, 0);
    initEffectKeeper(0, nullptr, false);
    initSound(6, "WaterPressureBullet", nullptr, TVec3f(0.0f));
    MR::initStarPointerTarget(this, 100.0f, TVec3f(0, 0, 0));
    MR::initShadowVolumeSphere(this, 75.0f);
    MR::setShadowDropLength(this, nullptr, 1500.0f);
    MR::registerDemoSimpleCastAll(this);
    initNerve(GET_NERVE(WaterPressureBullet, WaterPressureBulletNrvFly));
    mParamScale = new ActorStateParamScale(this);
    makeActorDead();
}

void WaterPressureBullet::kill() {
    if (MR::isPlayerInRush() && mHostActor != nullptr) {
        MR::startBckPlayer("GCaptureBreak", 0L);
        MR::endBindAndPlayerJumpWithRollLanding(this, mVelocity, 0);
        mHostActor = nullptr;
        endHostCamera();
    }

    MR::emitEffect(this, "Break");
    MR::startActionSound(this, "OjWPressBubbleBreak");
    LiveActor::kill();
}

#pragma push
#pragma opt_propagation off
void WaterPressureBullet::movement() {
    mParamScale->update();
    LiveActor::movement();
    mParamScale->resetHostVelocity();
}

void WaterPressureBullet::control() {
    if (isNotBound() && MR::isStarPointerPointing2PAttachTarget(this)) {
        kill();
    } else {
        TVec3f stack_8;

        if (MR::isNearZero(mVelocity)) {
            stack_8.set(*getGravity());
        } else {
            stack_8.set(*getVelocity());
        }

        MR::turnVecToVecCosOnPlane(&mFrontDir, stack_8, mSideDir, MR::cosDegree(2.5f));
        mParamScale->calcHostVelocity();
    }
}
#pragma pop

void WaterPressureBullet::calcAndSetBaseMtx() {
    TPos3f pos;
    MR::makeMtxFrontSidePos(&pos, mFrontDir, mSideDir, mPosition);
    MR::setBaseTRMtx(this, pos);
}

void WaterPressureBullet::shotWaterBullet(LiveActor* pActor, const TPos3f& rPos, f32 a3, bool a4, bool a5, bool a6, ActorCameraInfo** pInfo) {
    mSpeed = a3;
    mShooterActor = pActor;
    mIsNoGravity = a4;
    mCanSlideOnGround = a5;
    mIsLongLived = a6;
    mCameraInfo = pInfo;
    rPos.getZDir(mFrontDir);
    mVelocity.scale(mSpeed, mFrontDir);
    rPos.getTrans(mPosition);
    rPos.getXDir(mSideDir);
    mRotation.zero();
    makeActorAppeared();
    MR::validateHitSensors(this);
    MR::invalidateClipping(this);
    MR::setShadowDropLength(this, nullptr, 1500.0f);

    if (!mIsNoGravity) {
        MR::onCalcGravity(this);
    }

    setNerve(GET_NERVE(WaterPressureBullet, WaterPressureBulletNrvFly));
}

void WaterPressureBullet::shotWaterBullet(LiveActor* pActor, const ActorStateParamScale* pScale, const TPos3f& rPos, f32 a3, bool a4, bool a5, bool a6, ActorCameraInfo** pInfo) {
    shotWaterBullet(pActor, rPos, a3, a4, a5, a6, pInfo);
    mParamScale->init(pScale);
}

void WaterPressureBullet::exeFly() {
    if (mParamScale->isFirstStep()) {
        MR::startAction(this, "Shot");
    }

    if (MR::isBckOneTimeAndStopped(this)) {
        MR::startAction(this, "Move");
    }

    if (isBound() && MR::isBckOneTimeAndStopped(mHostActor)) {
        MR::startBckPlayer("WaterBulletWait", nullptr);
    }

    if (isBound()) {
        MR::startActionSound(this, "OjLvWPressBubbleSus");
    }

    if (!mIsNoGravity) {
        mParamScale->calcHostVelocity(mGravity, mVelocity, 0.4f);
    }

    if (MR::isPadSwing(WPAD_CHAN0) && isBound() && !mIsLongLived) {
        MR::startLevelSoundPlayer("TwistStart", -1, -1);
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        setNerve(GET_NERVE(WaterPressureBullet, WaterPressureBulletNrvSpinKill));
        return;
    }

    bool v2 = false;

    if (MR::isBinded(this) || MR::isInWater(this, TVec3f(0.0f, 0.0f, 0.0f))) {
        v2 = true;
    }

    if (v2) {
        if (mCanSlideOnGround && isBound() && MR::isBindedGroundSand(this)) {
            const TVec3f& vel = mVelocity;
            const TVec3f& grav = mGravity;
            mVelocity.scaleAdd(-grav.dot(vel), grav, vel);
        } else {
            kill();
            return;
        }
    }

    if (!mIsLongLived && mParamScale->isGreaterEqualStep(180) || mIsLongLived && mParamScale->isGreaterEqualStep(300)) {
        kill();
    }
}

void WaterPressureBullet::exeSpinKill() {
    if (mParamScale->isFirstStep()) {
        MR::startBckPlayer("Spin2nd", 0L);
        mVelocity.zero();
        MR::invalidateHitSensors(this);

        if (MR::isPlayerInRush()) {
            if (isBound()) {
                MR::endBindAndPlayerJump(this, mVelocity, 0);
                mHostActor = nullptr;
                endHostCamera();
            }
        }
    }

    kill();
}

void WaterPressureBullet::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrYoshiOrRide(pReceiver) && MR::isPlayerElementModeYoshi()) {
        kill();
    }
}

bool WaterPressureBullet::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgFireBallAttack(msg)) {
        kill();
        return true;
    }

    return false;
}

bool WaterPressureBullet::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isDead(this)) {
        return false;
    }

    if (MR::isMsgAutoRushBegin(msg) && MR::isSensorPlayer(pSender) && !isBound()) {
        if (MR::isDemoActive()) {
            kill();

            return false;
        }

        if (!inviteMario(pSender)) {
            return false;
        } else {
            MR::startActionSound(this, "OjWPressBubbleIn");
            MR::startLevelSoundPlayer("CatchV", -1, -1);

            return true;
        }
    } else if (MR::isMsgIsRushTakeOver(msg)) {
        return true;
    } else if (MR::isMsgRushCancel(msg)) {
        kill();

        return true;
    } else if (MR::isMsgUpdateBaseMtx(msg) && isBound()) {
        updateSuffererMtx();

        return true;
    }

    return false;
}

bool WaterPressureBullet::startHostCamera() const {
    if (mShooterActor != nullptr && mCameraInfo != nullptr) {
        MR::startActorCameraNoTarget(mShooterActor, *mCameraInfo, -1);
        return true;
    }

    return false;
}

bool WaterPressureBullet::endHostCamera() const {
    if (mShooterActor != nullptr && mCameraInfo != nullptr) {
        MR::endActorCamera(mShooterActor, *mCameraInfo, true, -1);
        return true;
    }

    return false;
}

// NONMATCHING: gravity/velocity address register lifetimes and saved registers differ.
bool WaterPressureBullet::inviteMario(HitSensor* pSensor) {
    MR::tryRumblePadMiddle(this, WPAD_CHAN0);

    if (MR::isOnGroundPlayer()) {
        if (!MR::isCalcGravity(this)) {
            MR::calcGravity(this);
        }
        if (MR::isNearAngleDegree(mVelocity, mGravity, 60.0f)) {
            if (mCanSlideOnGround) {
                mVelocity.killElement(mVelocity, mGravity);
            } else {
                kill();
                MR::sendMsgEnemyAttackFlipVeryWeak(pSensor, getSensor("body"));

                return false;
            }
        }
    }

    mHostActor = MR::getSensorHost(pSensor);
    MR::startBckWithInterpole(this, "Touch", 0);
    MR::startBckPlayer("WaterBulletStart", nullptr);
    startHostCamera();
    MR::setShadowDropLength(this, nullptr, 2000.0f);

    return true;
}

void WaterPressureBullet::updateSuffererMtx() {
    TPos3f pos;
    pos.identity();

    if (isNerve(GET_NERVE(WaterPressureBullet, WaterPressureBulletNrvSpinKill))) {
        TVec3f front;
        MR::calcFrontVec(&front, mHostActor);
        MR::calcMtxFromGravityAndZAxis(&pos, mHostActor, mGravity, front);
    } else {
        MtxPtr mtx = getBaseMtx();
        pos.setInline(mtx);
    }

    MR::setBaseTRMtx(mHostActor, pos);
}

WaterPressureBullet::~WaterPressureBullet() {
}
