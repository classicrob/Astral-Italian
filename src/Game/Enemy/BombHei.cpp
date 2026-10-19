#define JGEOMETRY_VEC3_ZERO_NOINLINE
#include "Game/Enemy/BombHei.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/StringUtil.hpp"

void BombHei_DUMMY() {
    TVec3f a;
    a *= 1.0f;
}

namespace {
    static const s32 hStartBrkTime = 600;
    static const s32 hBrkRateUpTime = 120;
    static const f32 hBrkSlowRate = 0.5f;
    static const f32 hBrkFastRate = 2.0f;
    static const f32 hTurnLimitRadian = 3.0f;
    static const f32 hGravity = 1.0f;
    static const f32 hGravityPhysicsOnGround = 0.5f;
    static const f32 hSpinHitJumpVel = 10.0f;
    static const f32 hPushedAcc = 2.0f;
    // static const f32 hNearDistance =  // TODO: some combination of these two is used in exeExplode
    // static const f32 hFarDistance =
    static const s32 hExplodeTime = 8;
    // static const s32 hHideTime =
    static const s32 hWaitTime = 180;
    static const s32 hWalkTime = 160;
    static const f32 hWalkVel = 0.5f;
    static const f32 hPursueVel = 1.5f;
    static const f32 hPursueVelFast = 2.0f;
    static const f32 hPrePursueJumpVel = 10.0f;
    // static const f32 hPursueHitJumpVel =
    // static const f32 hPursueHitBackVelH =
    static const s32 hPursueHitTime = 38;
    // static const f32 hRecoverRotateRate =
    static const f32 hGravityThrown = 0.75;
    static const f32 hThrownDampVel = 0.98f;
    static const f32 hThrownReboundRate = 0.8f;
    static const f32 hThrownReboundMin = 0.0f;
    // static const f32 hThrownToStopValue =
    static const f32 hTurnVecValue = 2.0f;
    static const s32 hThrownCanNotGotTime = 120;
    static const f32 hMultVelAngle = 1.0f;
    static const s32 hThrownOffCalcGravityTime = 30;
    static const f32 hGroundCosine = 0.75f;
    // static const s32 hStaticTime =
    static const f32 hStopDampValue = 0.8f;
    static const f32 hToStopVelLen = 5.0f;
    static const f32 hThrowVelH = 25.0f;
    static const f32 hThrowVelV = 18.0f;
    static const f32 hTranslateHeight = 53.0f;
    static const s32 hStopFrame = 3;
    static const s32 hSpinHitCanGetTime = 30;
    static const s32 hTrampleTime = 60;

    const BombHeiParam hNoMoveNoTurnParam = {0.0f, 2.0f, 0.8f, 0.0f};
    const BombHeiParam hWalkParam = {hWalkVel, 2.0f, 0.8f, hTurnVecValue};
    // const BombHeiParam hPursueParam =
    const BombHeiParam hPursueFarParam = {hPursueVel, 2.0f, 0.8f, hTurnVecValue + 1.0f};
    const BombHeiParam hPursueFastFarParam = {hPursueVelFast, 2.0f, 0.8f, hTurnVecValue + 1.0f};
    // const BombHeiParam hPursueNearParam =
    // const BombHeiParam hPursueWaitParam =
};  // namespace

namespace NrvBombHei {
    NEW_NERVE(HostTypeNrvLaunch, BombHei, Launch);
    NEW_NERVE(HostTypeNrvStarting, BombHei, Starting);
    NEW_NERVE(HostTypeNrvWait, BombHei, Wait);
    NEW_NERVE(HostTypeNrvWalk, BombHei, Walk);
    NEW_NERVE(HostTypeNrvPursue, BombHei, Pursue);
    NEW_NERVE(HostTypeNrvPursueFast, BombHei, Pursue);
    NEW_NERVE(HostTypeNrvPursueHit, BombHei, PursueHit);
    NEW_NERVE_ONEND(HostTypeNrvSpinHit, BombHei, SpinHit, SpinHit);
    NEW_NERVE(HostTypeNrvTrample, BombHei, Trample);
    NEW_NERVE(HostTypeNrvPhysics, BombHei, Physics);
    NEW_NERVE(HostTypeNrvStop, BombHei, Stop);
    NEW_NERVE_ONEND(HostTypeNrvTaken, BombHei, Taken, Taken);
    NEW_NERVE_ONEND(HostTypeNrvThrown, BombHei, Thrown, Thrown);
    NEW_NERVE(HostTypeNrvExplode, BombHei, Explode);
    NEW_NERVE(HostTypeNrvBindStarPointer, BombHei, BindStarPointer);

};  // namespace NrvBombHei

BombHei::BombHei(const char* pName)
    : LiveActor(pName), mScaleController(nullptr), mBindStarPointerState(nullptr), mRotQuat(0.0f, 0.0f, 0.0f, 1.0f), mFront(0, 0, 1),
      mBinderOffset(0.0f, ::hTranslateHeight, 0.0f), mTargetFront(0, 0, 1), mCountdown(-1), mCarrySensor(nullptr), mExplodeRadius(500.0f), mCountTimer(hStartBrkTime), mBodyJoint(nullptr),
      mType(BOMB_HEI), mExplosionIgnoreCollision(false) {
}

void BombHei::init(const JMapInfoIter& rIter) {
    if (MR::isValidInfo(rIter)) {
        MR::initDefaultPos(this, rIter);

        MR::getJMapInfoArg0NoInit(rIter, &mCountTimer);

        s32 rangeArg = mExplodeRadius;
        MR::getJMapInfoArg1NoInit(rIter, &rangeArg);
        mExplodeRadius = rangeArg;

        s32 ignoreCollisionArg = -1;
        MR::getJMapInfoArg2NoInit(rIter, &ignoreCollisionArg);
        if (ignoreCollisionArg != -1) {
            mExplosionIgnoreCollision = true;
        }

        const char* objName = nullptr;
        MR::getObjectName(&objName, rIter);
        if (MR::isEqualString(objName, "Bomb")) {
            mType = BOMB;
        } else {
            mType = BOMB_HEI;
        }

        setGroupClipping(rIter);
    }

    MR::initUseStageSwitchWriteDead(this, rIter);
    MR::initUseStageSwitchReadA(this, rIter);

    MR::initActor(this, rIter, "BombHei", false);
    MR::connectToSceneEnemy(this);
    MR::initLightCtrl(this);

    mRotQuat.set< f32 >(0.0f, 0.0f, 0.0f, 1.0f);
    mRotQuat.getZDir(mFront);
    mTargetFront.set(mFront);

    initBinder(::hTranslateHeight, ::hTranslateHeight, 0);
    MR::setBinderOffsetVec(this, &mBinderOffset, true);
    MR::onCalcGravity(this);

    initSound(4, "BombHei", nullptr, TVec3f(0.0f));
    initEffectKeeper(1, nullptr, false);

    initHitSensor(2);
    f32 scl = mScale.y;
    MR::addHitSensor(this, "Body", 0x1D /* ATYPE_BOMBHEI */, 16, scl * 60.0f, TVec3f(0.0f, scl * 60.0f, 0.0f));
    MR::addHitSensorEnemyAttack(this, "explode", 32, mExplodeRadius * scl, TVec3f(0.0f, scl * 200.0f, 0.0f));

    if (mType == BOMB) {
        initNerve(GET_NERVE(BombHei, HostTypeNrvPhysics), 1);
    } else {
        initNerve(GET_NERVE(BombHei, HostTypeNrvWait), 1);
    }

    MR::initShadowVolumeSphere(this, 60.0f);
    endCountdown();
    MR::validateClipping(this);
    MR::initStarPointerTarget(this, 70.0f, TVec3f(0.0f, 35.0f, 0.0f));

    mBodyJoint = MR::createJointDelegatorWithNullChildFunc(this, &BombHei::calcJoint, "Body");

    mScaleController = new AnimScaleController(nullptr);
    mScaleController->setParamTight();

    mBindStarPointerState = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mBindStarPointerState, GET_NERVE(BombHei, HostTypeNrvBindStarPointer), "SupportFreeze");

    appear();
}

void BombHei::setGroupClipping(const JMapInfoIter& rIter) {
    MR::setGroupClipping(this, rIter, 32);
}

void BombHei::initAfterPlacement() {
    MR::trySetMoveLimitCollision(this);
}

void BombHei::appear() {
    LiveActor::appear();
    MR::invalidateHitSensor(this, "explode");
    MR::validateHitSensor(this, "Body");

    if (mType == BOMB_HEI || mType == BOMB) {
        setNerve(GET_NERVE(BombHei, HostTypeNrvLaunch));
    }

    mCountdown = -1;
    MR::showModel(this);
}

void BombHei::kill() {
    LiveActor::kill();
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
    if (mCarrySensor != nullptr) {
        MR::sendArbitraryMsg(45, mCarrySensor, getSensor("Body"));
    }
    endCountdown();
}

void BombHei::startCountdown() {
    if (mCountdown == -1) {
        MR::invalidateClipping(this);
        mCountdown = mCountTimer;
        MR::emitEffect(this, "Ignite");
    }
}

void BombHei::endCountdown() {
    mCountdown = -1;
    MR::startBrk(this, "Wait");
    MR::setBrkRate(this, 1.0f);
    MR::deleteEffect(this, "Ignite");
    MR::validateClipping(this);
}

void BombHei::exeLaunch() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Stop");
        mRotQuat.set< f32 >(0.0f, 0.0f, 0.0f, 1.0f);
    }

    TVec3f velH(0, 0, 0);
    MR::vecKillElement(mVelocity, mGravity, &velH);
    mTargetFront.set(velH);
    MR::normalizeOrZero(&mTargetFront);

    if (!MR::isNearZero(mTargetFront)) {
        MR::turnVecToVecRadian(&mFront, mFront, mTargetFront, ::hTurnLimitRadian, mGravity);
    }

    mVelocity.mult(0.98f);

    if (!MR::isOnGroundCos(this, ::hGroundCosine)) {
        TVec3f grav(mGravity);
        grav *= ::hGravity;
        mVelocity.add(grav);
        TVec3f rollVel(velH);
        rollVel *= ::hMultVelAngle;
        f32 radius = MR::getSensorRadius(this, "Body");
        MR::rotateQuatRollBall(&mRotQuat, rollVel, -mGravity, radius);
    } else {
        startBoundSound();
        f32 radius = MR::getSensorRadius(this, "Body");
        MR::rotateQuatRollBall(&mRotQuat, mVelocity, MR::getGroundNormal(this), radius);

        if (MR::calcGravitySpeed(this) < 5.0f) {
            if (mType == BOMB_HEI) {
                setNerve(GET_NERVE(BombHei, HostTypeNrvStarting));
            } else if (mType == BOMB) {
                setNerve(GET_NERVE(BombHei, HostTypeNrvPhysics));
            }
            return;
        }
    }

    MR::reboundVelocityFromCollision(this, 0.1f, 6.0f, 1.0f);
}

void BombHei::exeStarting() {
    if (MR::isFirstStep(this)) {
        MR::validateExCollisionParts(this);
        MR::startAction(this, "Starting");
        mVelocity.zero();
    }

    if (!MR::isNearZero(mGravity)) {
        MR::blendQuatUpFront(&mRotQuat, -mGravity, mFront, 0.5f, 0.5f);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(BombHei, HostTypeNrvWait));
    }
}

#pragma global_optimizer off
void BombHei::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
    }

    MR::moveAndTurnToPlayer(this, &mFront, ::hNoMoveNoTurnParam.mSpeedH, ::hNoMoveNoTurnParam.mGravAccel, ::hNoMoveNoTurnParam.mFriction,
                            ::hNoMoveNoTurnParam.mTurnRate);

    if (MR::isGreaterStep(this, ::hWaitTime)) {
        setNerve(GET_NERVE(BombHei, HostTypeNrvWalk));
        return;
    }

    if (!MR::isNearZero(mGravity)) {
        MR::blendQuatUpFront(&mRotQuat, -mGravity, mFront, 0.5f, 0.5f);
    }

    if (MR::calcDistanceToPlayer(this) < 1000.0f && (!MR::isValidSwitchA(this) || MR::isOnSwitchA(this))) {
        if (mCountdown > 0 && mCountdown < ::hBrkRateUpTime) {
            setNerve(GET_NERVE(BombHei, HostTypeNrvPursueFast));
        } else {
            setNerve(GET_NERVE(BombHei, HostTypeNrvPursue));
        }
    }
}
#pragma global_optimizer on

// NONMATCHING: target keeps `mCountdown > 0 && mCountdown < 120` as two compares; CW merges them into one unsigned range check
void BombHei::exeWalk() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Walk");
        TVec3f randDir;
        MR::getRandomVector(&randDir, 1.0f);
        MR::vecKillElement(randDir, mGravity, &randDir);
        MR::normalizeOrZero(&randDir);
        if (!MR::isNearZero(randDir)) {
            mTargetFront.set(randDir);
        }
    }

    MR::moveAndTurnToDirection(this, &mFront, mTargetFront, ::hWalkParam.mSpeedH, ::hWalkParam.mGravAccel, ::hWalkParam.mFriction,
                               ::hWalkParam.mTurnRate);

    if (MR::isGreaterStep(this, ::hWalkTime)) {
        setNerve(GET_NERVE(BombHei, HostTypeNrvWait));
        return;
    }

    if (!MR::isNearZero(mGravity)) {
        MR::blendQuatUpFront(&mRotQuat, -mGravity, mFront, 0.5f, 0.5f);
    }

    if (MR::calcDistanceToPlayer(this) < 1000.0f && (!MR::isValidSwitchA(this) || MR::isOnSwitchA(this))) {
        if (mCountdown > 0 && mCountdown < ::hBrkRateUpTime) {
            setNerve(GET_NERVE(BombHei, HostTypeNrvPursueFast));
        } else {
            setNerve(GET_NERVE(BombHei, HostTypeNrvPursue));
        }
    }
}


#pragma global_optimizer off
void BombHei::exePursue() {
    if (MR::isFirstStep(this)) {
        startCountdown();
        MR::invalidateClipping(this);
        mVelocity.set(mGravity * -::hPrePursueJumpVel);
    }

    MR::calcDistanceToPlayer(this);

    if (isNerve(GET_NERVE(BombHei, HostTypeNrvPursueFast))) {
        MR::tryStartBck(this, "CountDown");
        MR::moveAndTurnToPlayer(this, &mFront, ::hPursueFastFarParam.mSpeedH, ::hPursueFastFarParam.mGravAccel, ::hPursueFastFarParam.mFriction,
                                ::hPursueFastFarParam.mTurnRate);
    } else {
        MR::tryStartBck(this, "Run");
        MR::moveAndTurnToPlayer(this, &mFront, ::hPursueFarParam.mSpeedH, ::hPursueFarParam.mGravAccel, ::hPursueFarParam.mFriction,
                                ::hPursueFarParam.mTurnRate);
    }

    if (isNerve(GET_NERVE(BombHei, HostTypeNrvPursue)) && mCountdown > 0 && mCountdown < ::hBrkRateUpTime) {
        setNerve(GET_NERVE(BombHei, HostTypeNrvPursueFast));
        return;
    }

    if (!MR::isNearZero(mGravity)) {
        MR::blendQuatUpFront(&mRotQuat, -mGravity, mFront, 0.5f, 0.5f);
    }
}
#pragma global_optimizer on

#pragma global_optimizer off
void BombHei::exePursueHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hit");
        MR::startActionSound(this, "EmBombheiHit");
    }

    MR::moveAndTurnToPlayer(this, &mFront, ::hNoMoveNoTurnParam.mSpeedH, ::hNoMoveNoTurnParam.mGravAccel, ::hNoMoveNoTurnParam.mFriction,
                            ::hNoMoveNoTurnParam.mTurnRate);

    if (!MR::isNearZero(mGravity)) {
        MR::blendQuatUpFront(&mRotQuat, -mGravity, mFront, 0.5f, 0.5f);
    }

    if (MR::isGreaterStep(this, ::hPursueHitTime)) {
        if (mCountdown > 0 && mCountdown < ::hBrkRateUpTime) {
            setNerve(GET_NERVE(BombHei, HostTypeNrvPursueFast));
        } else {
            setNerve(GET_NERVE(BombHei, HostTypeNrvPursue));
        }
    }
}
#pragma global_optimizer on

void BombHei::exeSpinHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Spin");
        MR::startBlowHitSound(this);
        mVelocity.set(mGravity * -::hSpinHitJumpVel);
        MR::invalidateExCollisionParts(this);
    }

    MR::moveAndTurnToPlayer(this, &mFront, ::hNoMoveNoTurnParam.mSpeedH, ::hNoMoveNoTurnParam.mGravAccel, ::hNoMoveNoTurnParam.mFriction,
                            ::hNoMoveNoTurnParam.mTurnRate);

    if (!MR::isNearZero(mGravity)) {
        MR::blendQuatUpFront(&mRotQuat, -mGravity, mFront, 0.5f, 0.5f);
    }

    if (MR::isOnGroundCos(this, ::hGroundCosine)) {
        if (MR::isActionEnd(this)) {
            setNerve(GET_NERVE(BombHei, HostTypeNrvPhysics));
        }
    } else {
        mVelocity.add(mGravity * ::hGravity);
    }
}

void BombHei::endSpinHit() {
}

void BombHei::exeTrample() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Pressed");
        MR::startActionSound(this, "EmBombheiTrample");
    }

    MR::addVelocityToGravityOrGround(this, ::hGravity);
    MR::killVelocityOnGroundCosH(this, ::hGroundCosine);

    if (MR::isGreaterStep(this, ::hTrampleTime)) {
        setNerve(GET_NERVE(BombHei, HostTypeNrvPursue));
    }
}

void BombHei::exePhysics() {
    if (MR::isFirstStep(this)) {
        MR::invalidateExCollisionParts(this);
    }

    TVec3f velH(0, 0, 0);
    MR::vecKillElement(mVelocity, mGravity, &velH);
    mTargetFront.set(velH);
    MR::normalizeOrZero(&mTargetFront);

    if (!MR::isNearZero(mTargetFront) && 2.0f < velH.length()) {
        MR::turnVecToVecRadian(&mFront, mFront, mTargetFront, ::hTurnLimitRadian, mGravity);
    }

    mVelocity.mult(0.98f);

    if (!MR::isOnGroundCos(this, ::hGroundCosine)) {
        TVec3f grav(mGravity);
        grav *= ::hGravity;
        mVelocity.add(grav);
        TVec3f rollVel(velH);
        rollVel *= ::hMultVelAngle;
        MR::rotateQuatRollBall(&mRotQuat, rollVel, mGravity, MR::getSensorRadius(this, "Body"));
    } else {
        mVelocity.add(mGravity * ::hGravityPhysicsOnGround);
        f32 radius = MR::getSensorRadius(this, "Body");
        MR::rotateQuatRollBall(&mRotQuat, mVelocity, MR::getGroundNormal(this), radius);

        if (MR::isActionEnd(this) && mVelocity.length() < ::hToStopVelLen) {
            setNerve(GET_NERVE(BombHei, HostTypeNrvStop));
            return;
        }

        mBinder->_1EC._5 = 1;
    }

    MR::reboundVelocityFromCollision(this, 0.1f, 6.0f, 1.0f);
    MR::killVelocityOnGroundCosH(this, ::hGroundCosine);
}

void BombHei::exeStop() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Stop");
    }

    if (MR::isBindedGround(this)) {
        MR::zeroVelocity(this);
    } else {
        MR::addVelocityToGravityOrGround(this, ::hGravity);
        MR::killVelocityOnGroundCosH(this, ::hGroundCosine);
    }

    if (MR::isOnGround(this)) {
        MR::attenuateVelocity(this, ::hStopDampValue);
        f32 radius = MR::getSensorRadius(this, "Body");
        MR::rotateQuatRollBall(&mRotQuat, mVelocity, MR::getGroundNormal(this), radius);
    } else {
        MR::attenuateVelocity(this, 0.98f);
        f32 radius = MR::getSensorRadius(this, "Body");
        MR::rotateQuatRollBall(&mRotQuat, mVelocity, -mGravity, radius);
    }

    mBinder->_1EC._5 = 1;
}

void BombHei::exeThrown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Stop");
        MR::startSoundPlayer("SE_PV_THROW", -1, -1);
        MR::startActionSound(this, "EmBombThrow");
        MR::invalidateExCollisionParts(this);
        MR::offCalcGravity(this);
    }

    TVec3f velH(0, 0, 0);
    MR::vecKillElement(mVelocity, mGravity, &velH);
    MR::rotateQuatRollBall(&mRotQuat, velH.multInLine(::hMultVelAngle), mGravity, MR::getSensorRadius(this, "Body"));
    mTargetFront.set(velH);
    MR::normalizeOrZero(&mTargetFront);

    if (!MR::isNearZero(mTargetFront)) {
        MR::turnVecToVecRadian(&mFront, mFront, mTargetFront, ::hTurnLimitRadian, mGravity);
    }

    if (MR::isGreaterStep(this, ::hThrownOffCalcGravityTime)) {
        MR::onCalcGravity(this);
    }

    mVelocity.mult(::hThrownDampVel);

    if (!MR::isOnGroundCos(this, ::hGroundCosine)) {
        mVelocity.add(mGravity * ::hGravityThrown);
    } else {
        startBoundSound();
        MR::startAction(this, "Bound");
        setNerve(GET_NERVE(BombHei, HostTypeNrvPhysics));
        return;
    }

    MR::reboundVelocityFromCollision(this, 0.1f, 6.0f, 1.0f);
}

void BombHei::endThrown() {
    MR::onCalcGravity(this);
}

void BombHei::exeTaken() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Carry");
        if (MR::sendMsgTaken(mCarrySensor, getSensor("Body"))) {
            MR::invalidateHitSensor(this, "Body");
            MR::startSoundPlayer("SE_PV_LIFT_UP", -1, -1);
            MR::startActionSound(this, "EmBombLift");
        } else {
            setNerve(GET_NERVE(BombHei, HostTypeNrvPhysics));
            return;
        }
    }

    mRotQuat.setEulerDegree(mRotation.x, mRotation.y, mRotation.z);
}

void BombHei::endTaken() {
    MR::validateHitSensor(this, "Body");
}

void BombHei::exeExplode() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "Explosion");
        MR::startActionSound(this, "EmBombExplosion");
        MR::hideModel(this);
        MR::validateHitSensor(this, "explode");
        MR::invalidateHitSensor(this, "Body");
        if (mCarrySensor != nullptr) {
            MR::sendArbitraryMsg(0x2D /* ACTMES_RUSHDROP */, mCarrySensor, getSensor("Body"));
            MR::sendMsgEnemyAttackExplosion(mCarrySensor, getSensor("Body"));
        }
        mCarrySensor = nullptr;
        endCountdown();
        MR::tryRumblePadAndCameraDistanceStrong(this, 800.0f, 1200.0f, 2000.0f);  // TODO
        MR::invalidateClipping(this);
    }

    MR::setSensorRadius(this, "explode", mExplodeRadius * getNerveStep() / 8);

    if (MR::isGreaterStep(this, ::hExplodeTime)) {
        MR::validateClipping(this);
        kill();
    }
}

void BombHei::exeBindStarPointer() {
    MR::updateActorStateAndNextNerve(this, GET_NERVE(BombHei, HostTypeNrvWait));
}

void BombHei::calcAndSetBaseMtx() {
    TPos3f mtx;
    MR::calcMtxFromGravityAndZAxis(&mtx, this, mGravity, mFront);
    MR::setBaseTRMtx(this, mtx);
    MR::updateBaseScale(this, mScaleController);
    mBodyJoint->registerCallBack();
}

bool BombHei::calcJoint(TPos3f* pMtx, const JointControllerInfo& rJointInfo) {
    TPos3f mtx;
    mtx.makeQuat(mRotQuat);

    // interesting way to transpose a matrix
    TPos3f mtx2;
    TVec3f x, y, z;
    mtx2.setInline(getBaseMtx());
    mtx2.getXYZDir(x, y, z);
    TPos3f mtx3;
    mtx3.setXYZDir2(x, y, z);

    TVec3f t;
    pMtx->getTrans(t);

    pMtx->zeroTransInline2();
    pMtx->concat(mtx3, *pMtx);
    pMtx->concat(mtx, *pMtx);
    pMtx->setTrans(t);

    return true;
}

void BombHei::control() {
    mScaleController->updateNerve();

    if (isNerve(GET_NERVE(BombHei, HostTypeNrvTaken))) {
        mBinderOffset.zero();
    } else {
        mBinderOffset.set(0.0f, ::hTranslateHeight, 0.0f);
    }

    if (MR::isInDeath(this, TVec3f(0.0f, 0.0f, 0.0f)) && !isNerve(GET_NERVE(BombHei, HostTypeNrvExplode))) {
        setNerve(GET_NERVE(BombHei, HostTypeNrvExplode));
        return;
    }

    if (mCountdown > 0) {
        if (mCountdown == mCountTimer) {
            MR::startBrk(this, "CountDown");
            MR::setBrkRate(this, ::hBrkSlowRate);
        } else if (mCountdown == ::hBrkRateUpTime) {
            MR::startBrk(this, "CountDown");
            MR::setBrkRate(this, ::hBrkFastRate);
        }

        MR::startActionSound(this, "EmLvBombFuse");

        if (mCountdown <= ::hBrkRateUpTime) {
            MR::startActionSound(this, "EmLvBombAlarmFast");
        } else if (mCountdown <= ::hStartBrkTime / 2) {
            MR::startActionSound(this, "EmLvBombAlarmMiddle");
        } else {
            MR::startActionSound(this, "EmLvBombAlarmSlow");
        }

        if (--mCountdown == 0) {
            setNerve(GET_NERVE(BombHei, HostTypeNrvExplode));
            return;
        }
    }

    if (mType != BOMB &&
        (isNerve(GET_NERVE(BombHei, HostTypeNrvWait)) || isNerve(GET_NERVE(BombHei, HostTypeNrvWalk)) ||
         isNerve(GET_NERVE(BombHei, HostTypeNrvPursue)) || isNerve(GET_NERVE(BombHei, HostTypeNrvPursueFast)) ||
         isNerve(GET_NERVE(BombHei, HostTypeNrvPursueHit))) &&
        mBindStarPointerState->tryStartPointBind()) {
        setNerve(GET_NERVE(BombHei, HostTypeNrvBindStarPointer));
    }
}

// NONMATCHING: stack slot order of the two TVec3f temporaries (0x8/0x14 swapped)
void BombHei::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (getSensor("explode") == pSender) {
        if (!mExplosionIgnoreCollision && MR::isExistCollisionParts(MR::getSensorHost(pReceiver)) &&
            MR::isValidCollisionParts(MR::getSensorHost(pReceiver))) {
            Triangle hitPoly;
            const TVec3f& rSenderPos = *MR::getSensorPos(pSender);
            TVec3f dir(*MR::getSensorPos(pReceiver));
            dir -= rSenderPos;

            if (!MR::getFirstPolyOnLineToMap(nullptr, &hitPoly, *MR::getSensorPos(pSender), dir)) {
                return;
            }

            HitSensor* pPartsSensor = hitPoly.mParts->mHitSensor;
            if (MR::getSensorHost(pPartsSensor) != MR::getSensorHost(pReceiver)) {
                return;
            }
        }

        if (MR::isSensorPlayer(pReceiver)) {
            MR::sendMsgEnemyAttackExplosion(pReceiver, pSender);
        } else if (MR::isSensorEnemy(pReceiver)) {
            MR::sendMsgEnemyAttackExplosion(pReceiver, pSender);
        } else if (MR::isSensorMapObj(pReceiver)) {
            MR::sendMsgEnemyAttackExplosion(pReceiver, pSender);
        }
        return;
    }

    if (isNerve(GET_NERVE(BombHei, HostTypeNrvLaunch)) || isNerve(GET_NERVE(BombHei, HostTypeNrvExplode))) {
        return;
    }

    if (getSensor("Body") != pSender) {
        return;
    }

    if (MR::isSensorPlayer(pReceiver)) {
        if (MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender)) {
            startCountdown();
        }
        return;
    }

    if (MR::isSensorRide(pReceiver)) {
        setNerve(GET_NERVE(BombHei, HostTypeNrvExplode));
        return;
    }

    if (!MR::isSensorNpc(pReceiver) && !MR::isSensorEnemy(pReceiver) && !MR::isSensorYoshi(pReceiver)) {
        return;
    }
    if (mCountdown != -1 && MR::sendArbitraryMsg(61, pReceiver, pSender)) {
        kill();
        return;
    }

    if (isNerve(GET_NERVE(BombHei, HostTypeNrvThrown))) {
        TVec3f direction(*MR::getSensorPos(pSender));
        direction -= *MR::getSensorPos(pReceiver);
        MR::normalizeOrZero(&direction);
        MR::calcReflectionVector(&mVelocity, direction, ::hThrownReboundRate, ::hThrownReboundMin);
    } else {
        MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
    }
}

bool BombHei::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(BombHei, HostTypeNrvExplode))) {
        return false;
    }

    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }

    if (MR::isMsgFireBallAttack(msg)) {
        if (!isNerve(GET_NERVE(BombHei, HostTypeNrvExplode))) {
            setNerve(GET_NERVE(BombHei, HostTypeNrvExplode));
        }
        return true;
    }

    if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
        if (isNerve(GET_NERVE(BombHei, HostTypeNrvThrown)) || isNerve(GET_NERVE(BombHei, HostTypeNrvPhysics)) ||
            isNerve(GET_NERVE(BombHei, HostTypeNrvStop)) || isNerve(GET_NERVE(BombHei, HostTypeNrvSpinHit))) {
            return false;
        }

        if (isNerve(GET_NERVE(BombHei, HostTypeNrvLaunch))) {
            if (MR::isMsgPlayerHipDrop(msg)) {
                MR::sendMsgAwayJump(pSender, pReceiver);
            }
            return true;
        }

        startCountdown();
        setNerve(GET_NERVE(BombHei, HostTypeNrvTrample));
        if (MR::isMsgPlayerHipDrop(msg)) {
            MR::sendMsgAwayJump(pSender, pReceiver);
        }
        return true;
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg) && mType == BOMB_HEI && !isNerve(GET_NERVE(BombHei, HostTypeNrvSpinHit))) {
        setNerve(GET_NERVE(BombHei, HostTypeNrvSpinHit));
        startCountdown();
        MR::stopScene(::hStopFrame);
        return true;
    }

    return false;
}

bool BombHei::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg) && !isNerve(GET_NERVE(BombHei, HostTypeNrvExplode))) {
        setNerve(GET_NERVE(BombHei, HostTypeNrvExplode));
        return true;
    }

    if ((MR::isMsgEnemyAttack(msg) || MR::isMsgToEnemyAttackBlow(msg) || MR::isMsgToEnemyAttackTrample(msg)) &&
        !isNerve(GET_NERVE(BombHei, HostTypeNrvExplode)) && MR::isSensorType(pSender, 0x30 /* ATYPE_WANWAN */)) {
        setNerve(GET_NERVE(BombHei, HostTypeNrvExplode));
        return true;
    }

    return false;
}

bool BombHei::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorPlayerOrYoshi(pSender) && !MR::isSensorEnemy(pSender) && !MR::isSensorNpc(pSender) && !MR::isSensorMapObj(pSender)) {
        return false;
    }

    if (isNerve(GET_NERVE(BombHei, HostTypeNrvSpinHit))) {
        return false;
    }

    MR::addVelocityFromPush(this, ::hPushedAcc, pSender, pReceiver);
    return true;
}

bool BombHei::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgThrow(msg)) {
        return receiveMsgThrow(pSender, pReceiver);
    }

    if (MR::isMsgItemGet(msg) && getSensor("Body") == pReceiver) {
        if (!isNerve(GET_NERVE(BombHei, HostTypeNrvStop)) && !isNerve(GET_NERVE(BombHei, HostTypeNrvPhysics)) &&
            !isNerve(GET_NERVE(BombHei, HostTypeNrvThrown)) &&
            !(isNerve(GET_NERVE(BombHei, HostTypeNrvSpinHit)) && MR::isGreaterStep(this, ::hSpinHitCanGetTime))) {
            return false;
        }

        if (isNerve(GET_NERVE(BombHei, HostTypeNrvThrown)) && MR::isLessEqualStep(this, ::hThrownCanNotGotTime)) {
            return false;
        }

        mCarrySensor = pSender;
        setNerve(GET_NERVE(BombHei, HostTypeNrvTaken));
        startCountdown();
        return true;
    }

    if (MR::isMsgDamageOrRushDrop(msg) || MR::isMsgWaterDrop(msg)) {
        if (isNerve(GET_NERVE(BombHei, HostTypeNrvTaken))) {
            setNerve(GET_NERVE(BombHei, HostTypeNrvThrown));
        }
        return true;
    }

    if (msg == 50) {
        MR::startAction(this, "Carry");
        return true;
    }

    return false;
}

bool BombHei::receiveMsgThrow(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(BombHei, HostTypeNrvExplode))) {
        return true;
    }

    setNerve(GET_NERVE(BombHei, HostTypeNrvThrown));
    mCarrySensor = nullptr;
    TVec3f throwVec;
    MR::getPlayerThrowVec(&throwVec);
    TVec3f velH(throwVec);
    velH *= ::hThrowVelH;
    mVelocity.set(velH);
    TVec3f velV(mGravity);
    velV *= -::hThrowVelV;
    mVelocity.add(velV);
    TVec3f back(mVelocity);
    back *= 3.0f;
    mPosition.sub(back);
    return true;
}

void BombHei::startBoundSound() {
    s32 level = MR::calcGravitySpeed(this) * 100.0f;
    MR::startActionSound(this, "EmBombBound", level);
}
