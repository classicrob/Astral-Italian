#pragma auto_inline off
#include "Game/MapObj/RollingWater.hpp"
#include "Game/MapObj/RollingWaterCreator.hpp"
#include "Game/Enemy/SupportTicoStop.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "JSystem/JMath/JMATrigonometric.hpp"

// PLACEHOLDER(objects): unnamed target symbol fn_800513C0, declared extern "C" so the reloc name matches; resolve when fn_800513C0 gets its real name in symbols.txt
extern "C" void fn_800513C0(TQuat4f*, const TVec3f*);
// PLACEHOLDER(objects): unnamed target symbol fn_800514B0, declared extern "C" so the reloc name matches; resolve when fn_800514B0 gets its real name in symbols.txt
extern "C" void fn_800514B0(TQuat4f*, const TVec3f*);

namespace NrvRollingWater {
    NEW_NERVE(Wait, RollingWater, Wait);
    NEW_NERVE(Ride, RollingWater, Ride);
    NEW_NERVE(Rush, RollingWater, Rush);
    NEW_NERVE(Fall, RollingWater, Fall);
}

RollingWater::RollingWater(const char* pName)
    : LiveActor(pName), mCreator(nullptr), mTicoStop(nullptr), mQuatBase(0.0f, 0.0f, 0.0f, 1.0f),
      mQuatTarget(0.0f, 0.0f, 0.0f, 1.0f), mQuatRail(0.0f, 0.0f, 0.0f, 1.0f) {
    mBlendVec.x = 0.0f;
    mBlendVec.y = 0.0f;
    mBlendVec.z = 0.0f;
    mCenterVec.x = 0.0f;
    mCenterVec.y = 0.0f;
    mCenterVec.z = 0.0f;
    mStartPos.x = 0.0f;
    mStartPos.y = 0.0f;
    mStartPos.z = 0.0f;
    mAddVel.x = 0.0f;
    mAddVel.y = 0.0f;
    mAddVel.z = 0.0f;
    mUpVec.x = 0.0f;
    mUpVec.y = 1.0f;
    mUpVec.z = 0.0f;
    mTimer = 300;
    mRadius = 1.0f;
    mRailSpeed = 15.0f;
    mSpeedRate = 0.0f;
    mAngVel = 0.0f;
    mStepCount = 0;
    mIsTouched = false;
}


void RollingWater::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "RollingWater", false);
    mRadius = mScale.y;
    mStartPos.set(mPosition);
    MR::makeQuatFromRotate(&mQuatTarget, this);
    mQuatBase = mQuatTarget;
    if (MR::isConnectedWithRail(rIter)) {
        MR::moveCoordAndTransToStartPos(this);
        MR::getJMapInfoArg0NoInit(rIter, &mRailSpeed);
    }
    mTicoStop = new SupportTicoStop(this, nullptr);
    initNerve(GET_NERVE(RollingWater, Wait), 0);
    makeActorAppeared();
    MR::startAction(this, "Bound");
    MR::startBtk(this, "Size");
}

void RollingWater::kill() {
    if (mCreator != nullptr) {
        MR::sendSimpleMsgToActor(71, mCreator);
    }
    LiveActor::kill();
}

void RollingWater::control() {
    mTicoStop->updateNerve();
    if (mIsTouched) {
        s32 step = getNerveStep();
        f32 t = static_cast< f32 >(step % 6);
        f32 phase = 3.1415927f * (2.0f * t) / 6.0f;
        if (phase < 0.0f) {
            phase = -phase;
        }
        s32 index = static_cast< s32 >(2607.594482421875f * phase);
        f32 cosVal = *reinterpret_cast< const f32* >(reinterpret_cast< const u8* >(&JMath::sSinCosTable) + ((index * 8) & 0x1FFF8) + 4);
        mIsTouched = false;
        mSpeedRate = 0.5f + 0.1f * cosVal;
    } else {
        mSpeedRate += mAngVel;
        mAngVel *= 0.98f;
    }

    if (mSpeedRate >= 0.0f) {
        if (mSpeedRate > 1.0f) {
            mSpeedRate = 1.0f;
        }
    } else {
        mSpeedRate = 0.0f;
    }

    MR::setBckFrameAndStop(this, MR::getBckFrameMax(this) * mSpeedRate);

    f32 t = 0.0f;
    if (mRadius > 1.0f) {
        if (mRadius <= 1.5f) {
            t = MR::normalize(mRadius, 1.0f, 1.5f);
        } else if (mRadius <= 2.0f) {
            t = 1.0f + MR::normalize(mRadius, 1.5f, 2.0f);
        }
    }
    MR::setBtkFrameAndStop(this, t);

    if (MR::isBindedGround(this)) {
        mUpVec.set(MR::getGroundNormal(this));
        mStepCount = 0;
    } else {
        mStepCount++;
    }

    if (mStepCount > 5) {
        TVec3f negGravity = -mGravity;
        mUpVec.set(negGravity);
    }
}

void RollingWater::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuatBase);
}

void RollingWater::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::sendArbitraryMsg(69, pReceiver, pSender)) {
        checkPlayerAttack();
    }
}

bool RollingWater::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isMsgSupportTicoSpin(msg)) {
        return false;
    }

    if (!isNerve(GET_NERVE(RollingWater, Wait)) && isNerve(GET_NERVE(RollingWater, Ride))) {
        MR::tryHitReactionBreak(this);
        kill();
        return true;
    }

    if (isNerve(GET_NERVE(RollingWater, Rush)) && MR::isGreaterStep(this, 20)) {
        setNerve(GET_NERVE(RollingWater, Fall));
        return true;
    }

    return false;
}

bool RollingWater::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 69) {
        return checkPlayerAttack();
    }

    if (MR::isMsgAutoRushBegin(msg) && isNerveRide()) {
        f32 dist = MR::calcDistance(pSender, pReceiver, nullptr);
        if (dist <= 30.0f + 125.0f * mRadius) {
            fn_800514B0(&mQuatRail, &mCenterVec);
            TVec3f diff = mCenterVec - mPosition;
            mBlendVec.set(diff);
            setNerve(GET_NERVE(RollingWater, Rush));
            if (mCreator != nullptr) {
                MR::sendSimpleMsgToActor(70, mCreator);
                return true;
            }
        }
    }

    if (MR::isMsgRushCancel(msg)) {
        MR::tryHitReactionBreak(this);
        kill();
        return true;
    }

    if (MR::isMsgUpdateBaseMtx(msg)) {
        fn_800513C0(&mQuatRail, &mCenterVec);
        return true;
    }

    return false;
}

#pragma dont_inline on
bool RollingWater::isNerveRide() const {
    return isNerve(GET_NERVE(RollingWater, Ride));
}

#pragma dont_inline reset

bool RollingWater::moveRoute() {
    if (MR::isExistRail(this)) {
        return moveOnRail();
    }
    return updateTimerAndCheckArea();
}

void RollingWater::updateVelocityAndRotation() {
    if (MR::reboundVelocityFromEachCollision(this, 0.5f, 0.0f, 0.0f, 12.0f) && MR::isBindedGround(this)) {
        MR::turnQuatYDirRate(&mQuatBase, mQuatBase, mUpVec, 0.5f);
        mAngVel = 0.15f;
        MR::tryHitReactionAtPos(this, "\x92\x85\x92\x6E", &MR::getBindedHitPos(this));
        s32 pitch = static_cast< s32 >(1000.0f * MR::calcGravitySpeedAbs(this));
        MR::startActionSound(this, "Bound", pitch, -1, -1);
    } else if (mStepCount <= 5) {
        MR::turnQuatYDirRate(&mQuatBase, mQuatBase, mUpVec, 0.1f);
        mAngVel += 0.1f * (0.36f - mSpeedRate);
    } else {
        MR::turnQuatYDirRate(&mQuatBase, mQuatBase, mUpVec, 0.1f);
        mAngVel += 0.1f * (0.1f - mSpeedRate);
    }

    MR::addVelocityToGravity(this, 1.5f);
    f32 attenuate;
    if (MR::isBindedGround(this)) {
        attenuate = 0.98f;
    } else {
        attenuate = 0.995f;
    }
    MR::attenuateVelocity(this, attenuate);
}

bool RollingWater::updateTimerAndCheckArea() {
    updateVelocityAndRotation();
    updateRollMoment();
    mTimer--;
    if (MR::isInAreaObj("RollingWaterBreakArea", mPosition)) {
        return true;
    }
    return mTimer <= 0;
}

bool RollingWater::moveOnRail() {
    updateVelocityAndRotation();
    MR::killVelocityHorizon(this);
    MR::moveCoord(this, mRailSpeed);
    TVec3f railVec;
    MR::calcVecToTargetHorizon(&railVec, this, &MR::getRailPos(this), 0);
    MR::addVelocity(this, railVec);
    updateRollMoment();
    return MR::isRailReachedGoal(this);
}

void RollingWater::updateRollMoment() {
    TVec3f moment;
    TVec3f negGravity = -mGravity;
    MR::calcMomentRollBall(&moment, mVelocity, negGravity, 125.0f);
    f32 ease = MR::calcNerveEaseInOutRate(this, 40);
    TVec3f a(moment);
    a *= 0.2f;
    TVec3f b(a);
    b *= ease;
    mAddVel += b;
    mAddVel.x *= 0.9f;
    mAddVel.y *= 0.9f;
    mAddVel.z *= 0.9f;
    MR::rotateQuatMoment(&mQuatRail, mAddVel);
}

void RollingWater::resetToStart() {
    if (MR::isExistRail(this)) {
        MR::moveCoordAndTransToStartPos(this);
        MR::resetPosition(this);
    } else {
        MR::resetPosition(this, mStartPos);
    }

    MR::zeroVelocity(this);
}

void RollingWater::appearFromCreator(LiveActor* pCreator, s32 timer) {
    mCreator = pCreator;
    mTimer = timer;
    resetToStart();
    MR::showModel(this);
    setNerve(GET_NERVE(RollingWater, Ride));
    appear();
    TVec3f negGravity = -mGravity;
    mUpVec.set(negGravity);
}

s32 RollingWater::calcRailStepCount() const {
    s32 zero = 0;
    s32 steps;
    if (MR::isExistRail(this)) {
        f32 total = MR::getRailTotalLength(this);
        steps = static_cast< s32 >(static_cast< f32 >(zero) + (2.0f + total / mRailSpeed));
    } else {
        steps = 302;
    }
    return steps + 7;
}

void RollingWater::exeWait() {
    if (MR::isFirstStep(this)) {
        resetToStart();
        MR::hideModel(this);
    }
    if (MR::isGreaterStep(this, 120)) {
        MR::showModel(this);
        MR::startActionSound(this, "Appear", -1, -1, -1);
        setNerve(GET_NERVE(RollingWater, Ride));
    }
}

void RollingWater::exeRide() {
    if (MR::isFirstStep(this)) {
        mAddVel.z = 0.0f;
        mAddVel.y = 0.0f;
        mAddVel.x = 0.0f;
    }
    if (tryAttachTicoTouch()) {
        return;
    }
    if (moveRoute()) {
        MR::tryHitReactionBreak(this);
        kill();
    }
}

void RollingWater::exeRush() {
    if (MR::isFirstStep(this)) {
        if (MR::isPlayerElementModeBee() || MR::isPlayerElementModeCloud()) {
            MR::curePlayerElementMode();
        }
        mAngVel = -0.15f;
        MR::startBckPlayer("SpaceWait", nullptr);
        MR::startLevelSoundPlayer("RollInWater", -1, -1);
        MR::tryHitReactionAtPos(this, "\x8A\xAA\x82\xAB\x8D\x9E\x82\xDD", MR::getPlayerCenterPos());
    }

    f32 rate = MR::calcNerveEaseOutRate(this, 60);
    TVec3f zero(0.0f, 0.0f, 0.0f);
    MR::vecBlend(&mBlendVec, mBlendVec, zero, rate);
    TVec3f center(mPosition);
    center += mBlendVec;
    mCenterVec.set(center);

    if (MR::isGreaterStep(this, 20) && MR::isPadSwing(0)) {
        setNerve(GET_NERVE(RollingWater, Fall));
    } else if (!tryAttachTicoTouch()) {
        if (moveRoute()) {
            setNerve(GET_NERVE(RollingWater, Fall));
        }
    }
}

void RollingWater::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::startBckPlayer("Spin2nd", "GCaptureCancel");
        MR::startLevelSoundPlayer("RollOutWater", -1, -1);
        MR::tryHitReactionBreak(this);
    }

    TVec3f prevCenter(mCenterVec);
    moveRoute();
    MR::turnQuatUpToGravity(&mQuatRail, mQuatRail, this);

    f32 rate = MR::calcNerveEaseOutRate(this, 60);
    TVec3f zero(0.0f, 0.0f, 0.0f);
    MR::vecBlend(&mBlendVec, mBlendVec, zero, rate);
    TVec3f center(mPosition);
    center += mBlendVec;
    mCenterVec.set(center);

    if (MR::isGreaterStep(this, 5)) {
        TVec3f diff = mCenterVec - prevCenter;
        f32 dot = mGravity.dot(diff);
        TVec3f out;
        JMAVECScaleAdd(&mGravity, &diff, &out, -dot);
        TVec3f force(mGravity);
        force.x *= -15.0f;
        force.y *= -15.0f;
        force.z *= -15.0f;
        MR::endBindAndPlayerForceWeakGravityJumpInputOff(this, force);
        kill();
    }
}

bool RollingWater::tryAttachTicoTouch() {
    if (mTicoStop->tryAttachTouch()) {
        mAngVel = 0.0f;
        MR::zeroVelocity(this);
        mAddVel.x *= 0.99f;
        mAddVel.y *= 0.99f;
        mAddVel.z *= 0.99f;
        mIsTouched = true;
        return true;
    }
    return false;
}

bool RollingWater::checkPlayerAttack() {
    if (isNerve(GET_NERVE(RollingWater, Wait))) {
        return false;
    }
    if (isNerve(GET_NERVE(RollingWater, Ride))) {
        MR::tryHitReactionBreak(this);
        kill();
        return true;
    }
    if (isNerve(GET_NERVE(RollingWater, Rush))) {
        setNerve(GET_NERVE(RollingWater, Fall));
        return true;
    }
    return false;
}

namespace MR {
    bool isMsgRollingWaterPlayerEnter(u32 msg) {
        return msg == 70;
    }

    bool isMsgRollingWaterPlayerExit(u32 msg) {
        return msg == 71;
    }
}

RollingWater::~RollingWater() {
}
