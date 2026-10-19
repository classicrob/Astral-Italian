#define JGEOMETRY_OUTLINE_MATRIX_POSITION
#define JGEOMETRY_VEC3_WMR_OUTLINE
#define JGEOMETRY_QUAT_TRANSFORM_NOINLINE
#define JGEOMETRY_VEC3_NEG_NOINLINE
#include "Game/MapObj/ItemDiceFunction.hpp"
#pragma auto_inline off

#include "Game/MapObj/DiceBase.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "JSystem/JGeometry/TMatrix.hpp"
#include "JSystem/JGeometry/TVec.hpp"
#include "JSystem/JMath/JMath.hpp"
#include <cmath>

// Bounding spheres of the dice (x, y, z, radius).
static f32 sDiceSpheres[9][4] = {
    {0.0f, 0.0f, 0.0f, 100.0f}, {0.0f, 0.0f, 0.0f, 20.0f}, {0.0f, 0.0f, 0.0f, 20.0f}, {0.0f, 0.0f, 0.0f, 20.0f},
    {0.0f, 0.0f, 0.0f, 20.0f},  {0.0f, 0.0f, 0.0f, 20.0f}, {0.0f, 0.0f, 0.0f, 20.0f}, {0.0f, 0.0f, 0.0f, 20.0f},
    {0.0f, 0.0f, 0.0f, 20.0f},
};


namespace NrvItemDice {
    NEW_NERVE(ItemDiceNrvSpin, DiceBase, Spin);
    NEW_NERVE(ItemDiceNrvThrow, DiceBase, Throw);
    NEW_NERVE(ItemDiceNrvLand, DiceBase, Land);
};  // namespace NrvItemDice

DiceBase::DiceBase(const char* pName) : LiveActor(pName) {
    mMover = nullptr;
    mBasePos.x = 0.0f;
    mBasePos.y = 0.0f;
    mBasePos.z = 0.0f;
    mBaseRot.x = 0.0f;
    mBaseRot.y = 0.0f;
    mBaseRot.z = 0.0f;
    mThrowScaleA = 18.0f;
    mThrowScaleB = 18.0f;
    mStopTimer = 0;
    mLandTimer = 0;
    mBounceCount = 0;
    mHitCount = 0;
    mUnkF4 = 0;
    mUnkF8 = 0;
    mSpinVel.x = 0.0f;
    mSpinVel.y = 0.0f;
    mSpinVel.z = 0.0f;
    mIsLanded = false;
    mIsGood = false;
}

// NONMATCHING: 96%: stack slots of the two JMap temporaries differ (same calls and constants).
void DiceBase::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);

    s32 arg1;
    MR::getJMapInfoArg1WithInit(rIter, &arg1);
    if (arg1 != -1) {
        mThrowScaleA = (f32)arg1 / 10.0f;
    }

    s32 arg2;
    MR::getJMapInfoArg2WithInit(rIter, &arg2);
    if (arg2 != -1) {
        mThrowScaleB = (f32)arg2 / 10.0f;
    }

    initEffectKeeper(0, "ItemDice", false);
    MR::initActor(this, rIter, getModelName(), false);
    initHitSensor(1);
    HitSensor* pSensor = MR::addHitSensorPush(this, "Body", 0x20, 142.0f, TVec3f(0.0f, 0.0f, 0.0f));
    initSound(4, "ItemDice", nullptr, TVec3f(0.0f));
    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
    MR::connectToSceneNoShadowedMapObj(this);
    initNerve(GET_NERVE(ItemDice, ItemDiceNrvSpin), 0);
    MR::invalidateClipping(this);
    mBasePos.set(mPosition);
    mBaseRot.set(mRotation);
    MR::onCalcGravity(this);
    makeActorAppeared();
    createMover(pSensor);

    TVec3f zero;
    zero.x = 0.0f;
    zero.y = 0.0f;
    zero.z = 0.0f;
    MR::initStarPointerTarget(this, 145.0f, zero);
    MR::useStageSwitchAwake(this, rIter);
    initItems(rIter);
}

void DiceBase::appear() {
    LiveActor::appear();
    resetToBaseTrans();
    LiveActor::calcAndSetBaseMtx();
    setNerve(GET_NERVE(ItemDice, ItemDiceNrvSpin));
    MR::hideModel(this);
    MR::invalidateClipping(this);
    mStopTimer = 0;
    mLandTimer = 0;
    mBounceCount = 0;
    mHitCount = 0;
    mIsLanded = false;
    mIsGood = false;
}

// NONMATCHING: same calls/constants/branches; the M1/M2 matrix float ops are scheduled and register-allocated differently (64%).
// NONMATCHING: second rotation matrix element scheduling (fneg/fmuls order); first matrix is the inline TRotation3::setRotate
void DiceBase::exeSpin() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::showModel(this);
        setAnimFrame();
        mSpinVel.x = 45.0f;
        mSpinVel.y = 0.0f;
        mSpinVel.z = 45.0f;
    }

    f32 kDeg2Rad = 0.01745329238474369f;
    f32 kRad2Deg = 57.2957763671875f;

    // M1 = Rz(z) * Ry(y) * Rx(x) from mBaseRot (degrees)
    TRot3f mtx1;
    mtx1.identity();
    TVec3f ang1(mBaseRot);
    ang1.scale(kDeg2Rad);
    mtx1.setRotate(ang1);

    // M2 from mSpinVel (degrees); element formulas transcribed from the target asm
    TRot3f mtx2;
    mtx2.identity();
    TVec3f ang2(mSpinVel);
    ang2.scale(kDeg2Rad);
    f32 z2 = ang2.z;
    f32 y2 = ang2.y;
    f32 x2 = ang2.x;
    f32 cz2 = (f32)cos(z2);
    f32 cy2 = (f32)cos(y2);
    f32 cx2 = (f32)cos(x2);
    f32 sz2 = (f32)sin(z2);
    f32 sy2 = (f32)sin(y2);
    f32 sx2 = (f32)sin(x2);
    mtx2.mMtx[1][0] = sz2;
    mtx2.mMtx[2][0] = (-sy2) * cz2;
    mtx2.mMtx[1][1] = cx2 * cz2;
    mtx2.mMtx[0][2] = cx2 * sy2 + sx2 * cy2 * sz2;
    mtx2.mMtx[0][0] = cy2 * cz2;
    mtx2.mMtx[2][1] = sx2 * cy2 + cx2 * sy2 * sz2;
    mtx2.mMtx[2][2] = (cx2 * cy2) + sz2 * ((-sx2) * sy2);
    mtx2.mMtx[1][2] = (-sx2) * cz2;
    mtx2.mMtx[0][1] = (-(cx2 * cy2)) * sz2 + sx2 * sy2;

    TRot3f mtx3;
    mtx3.concat(mtx1, mtx2);
    mtx3.getEulerXYZ(mRotation);
    mRotation.scale(kRad2Deg);

    mSpinVel.add(TVec3f(0.0f, 2.0f, 0.0f));
    MR::attachSupportTicoToTarget(this);
}

// NONMATCHING: 94%: register choice for the `bound` local and -1 argument load order.
void DiceBase::exeThrow() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::startActionSound(this, "Thrown", -1, -1, -1);
    }

    TVec3f vel(mGravity);
    vel.scale(0.3499999940395355f);
    mMover->setGravity(&vel);
    mMover->update();

    if (isSettled()) {
        setNerve(GET_NERVE(ItemDice, ItemDiceNrvLand));
        return;
    }

    if (mLandTimer != 0) {
        mLandTimer--;
    }
    if (mLandTimer == 0) {
        setNerve(GET_NERVE(ItemDice, ItemDiceNrvLand));
        return;
    }
    if (mHitCount >= 2) {
        setNerve(GET_NERVE(ItemDice, ItemDiceNrvLand));
        return;
    }

    if (mMover->mD5) {
        f32 d = mMover->mD8 - 0.699999988079071f;
        s32 bound;
        if (d > 0.0f) {
            bound = (s32)(200.0f + 200.0f * (d * d));
        } else {
            bound = 0;
        }
        if (bound > 0x258) {
            MR::startActionSound(this, "Bound", bound, -1, -1);
        } else if (bound > 0) {
            MR::startActionSound(this, "BoundLittle", bound, -1, -1);
        }
    }

    MR::startActionSound(this, "ThrownLv", -1, -1, -1);
    if (MR::isGreaterStep(this, 27)) {
        MR::startActionSound(this, "Rolling", -1, -1, -1);
    }
}

// NONMATCHING: 96%: string-table base is hoisted into a register differently (same calls and constants).
void DiceBase::exeLand() {
    if (MR::isFirstStep(this)) {
        const TVec3f* pVel = &TVec3f(mGravity);
        s32 result = ItemDiceFunction::calcFaceIndex(getBaseMtx(), pVel);
        if (isGoodResult(result)) {
            MR::startActionSound(this, "FixedGood", -1, -1, -1);
        } else {
            MR::startActionSound(this, "FixedBad", -1, -1, -1);
        }
    }

    if (MR::isStep(this, 10)) {
        const TVec3f* pVel = &TVec3f(mGravity);
        s32 result = ItemDiceFunction::calcFaceIndex(getBaseMtx(), pVel);
        setResultFrame(result);
    }

    if (MR::isGreaterEqualStep(this, 40)) {
        MR::emitEffect(this, "Explosion");
        mIsLanded = true;

        const TVec3f* pVel = &TVec3f(mGravity);
        s32 result = ItemDiceFunction::calcFaceIndex(getBaseMtx(), pVel);
        mIsGood = makeResult(result);
        if (mIsGood) {
            MR::startActionSound(this, "VanishGood", -1, -1, -1);
        } else {
            MR::startActionSound(this, "VanishBad", -1, -1, -1);
        }
        kill();
    }
}

void DiceBase::control() {
    if (mStopTimer != 0) {
        mStopTimer--;
    }
}

void DiceBase::calcAndSetBaseMtx() {
    if (isNerve(GET_NERVE(ItemDice, ItemDiceNrvThrow))) {
        TPos3f mtx;
        mMover->calcMtx(&mtx);
        MR::setBaseTRMtx(this, mtx);
        mtx.getTrans(mPosition);
        mtx.getEulerXYZ(mRotation);
        mRotation.x *= 57.2957763671875f;
        mRotation.y *= 57.2957763671875f;
        mRotation.z *= 57.2957763671875f;
    } else {
        LiveActor::calcAndSetBaseMtx();
    }
}

void DiceBase::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorType(pReceiver, 0x2D)) {
        MR::sendMsgEnemyAttackExplosion(pReceiver, pSender);
    } else if (isPushableSensor(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool DiceBase::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pSender) && !MR::isPlayerElementModeRock()) {
        return false;
    }

    receiveMsgPushImpl(pSender, pReceiver, 0.008000000379979610f);
    return true;
}

// Target outlines vector subtraction and matrix translation at different boundaries.
bool DiceBase::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg) || MR::isMsgSnowBallAttack(msg) || MR::isMsgFireBallAttack(msg) ||
        MR::isMsgYoshiProjectileOrEat(msg) || MR::isMsgRockAttack(msg)) {
        if (isNerve(GET_NERVE(ItemDice, ItemDiceNrvSpin))) {
            setNerve(GET_NERVE(ItemDice, ItemDiceNrvThrow));
            MtxPtr pMtx = getBaseMtx();
            mMover->setMtx(pMtx);
            startHitStop();
            TVec3f up;
            MR::getPlayerUpVec(&up);
            calcPushVec(*MR::getSensorPos(pSender), up);
            MR::tryHitReaction(this, "\222e\203q\203b\203g");
        }
        return true;
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        if (tryThrow()) {
            return false;
        }
        if (!isNerve(GET_NERVE(ItemDice, ItemDiceNrvThrow))) {
            setNerve(GET_NERVE(ItemDice, ItemDiceNrvThrow));
            MtxPtr pMtx = getBaseMtx();
            TPos3f mtx;
            JMath::gekko_ps_copy12(&mtx, pMtx);
            TVec3f vel(mGravity);
            vel.scale(10.0f);
            TVec3f pos = mPosition - vel;
            mtx.setTrans(pos);
            mMover->setMtx(mtx.toMtxPtr());
            MR::stopSceneForDefaultHit(6);
            MR::tryRumblePadStrong(this, 0);
        }
        MR::tryHitReaction(this, "\222\274\220\332\203q\203b\203g");
        startHitStop();
        TVec3f up;
        MR::getPlayerUpVec(&up);
        calcPushVec(*MR::getSensorPos(pSender), up);
        return true;
    }

    if (MR::isMsgPlayerTrample(msg)) {
        MR::startActionSound(this, "Trampled", -1, -1, -1);
        return true;
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        MR::sendMsgAwayJump(pSender, pReceiver);
        return true;
    }

    return false;
}

bool DiceBase::tryThrow() {
    bool blocked = false;
    if (mStopTimer != 0 || isNerve(GET_NERVE(ItemDice, ItemDiceNrvLand)) || mHitCount >= 2) {
        blocked = true;
    }
    return blocked;
}

void DiceBase::resetToBaseTrans() {
    MR::setPosition(this, mBasePos);
    MR::setRotation(this, mBaseRot);
    mMover->mSpeedLimit = 200.0f;
}

// NONMATCHING: calls match; field stores reload mMover in a different order than the target.
void DiceBase::createMover(HitSensor* pSensor) {
    DiceMover* pMover = new DiceMover(0, 0);
    mMover = pMover;
    mMover->mSpeedLimit = 200.0f;
    mMover->mB4 = 20.0f;
    mMover->m8C = 2.0f;
    mMover->mSensorDC = pSensor;
    mMover->mE0 = true;

    MtxPtr pMtx = getBaseMtx();
    mMover->setMtx(pMtx);

    struct SphereArray {
        s32 count;
        f32 (*spheres)[4];
    };
    SphereArray* pBlock = new SphereArray;
    if (pBlock) {
        pBlock->count = 9;
        pBlock->spheres = sDiceSpheres;
    }
    mMover->setPointArray(pBlock, 0);
}

void DiceBase::calcPushVec(const TVec3f& rPos, const TVec3f& rUp) {
    MtxPtr pMtx = getBaseMtx();
    TVec3f dir = rPos - mPosition;
    if (MR::normalizeOrZero(&dir)) {
        dir.set<f32>(pMtx[0][0], pMtx[1][0], pMtx[2][0]);
    }

    f32 dot = rUp.dot(dir);
    TVec3f proj;
    JMAVECScaleAdd(&rUp, &dir, &proj, -dot);

    if (!MR::normalizeOrZero(&proj)) {
        f32 kAng = -0.7853981852531433f;
        TQuat4f quat;
        TVec3f& quatAxis = *reinterpret_cast<TVec3f*>(&quat);
        quatAxis.scale(sin(kAng), rUp);
        quat.w = (f32)cos(kAng);
        TVec3f axis;
        quat.transform(proj);
        PSVECCrossProduct(&proj, &rUp, &axis);
        MR::normalizeOrZero(&axis);
        proj.scale(100.0f);
        proj.add(mPosition);
        f32 scaleA = mThrowScaleA;
        TVec3f tmp(axis);
        tmp.scale(scaleA);
        mMover->collide(&proj, &tmp);
    }

    TVec3f pos = rPos - mPosition;
    MR::normalizeOrZero(&pos);
    pos.scale(50.0f);
    pos.add(mPosition);
    f32 scaleB = mThrowScaleB;
    TVec3f grav = -mGravity;
    TVec3f vel = grav;
    vel.scale(scaleB);
    mMover->collide(&pos, &vel);
}

// NONMATCHING: float constant scheduling around 2.0f*acos / fabs compare (instruction set identical)
bool DiceBase::isSettled() {
    if (MR::isFirstStep(this)) {
        MtxPtr pMtx = getBaseMtx();
        JMath::gekko_ps_copy12(&mMtxC4, pMtx);
        mBounceCount = 0;
    }

    MtxPtr pMtx = getBaseMtx();
    TPos3f mtx;
    JMath::gekko_ps_copy12(&mtx, pMtx);

    TVec3f trans1;
    mMtxC4.getTrans(trans1);
    TVec3f trans2;
    mtx.getTrans(trans2);
    mtx.invert(mtx);
    mtx.concat(mMtxC4);

    TVec3f axis;
    TQuat4f quat;
    mtx.getQuat(quat);
    f32 sq = reinterpret_cast<const TVec3f*>(&quat)->squared();

    if (sq <= 0.000003814697265625f) {
        axis.x = axis.y = axis.z = 0.0f;
    } else {
        axis.scale(JGeometry::TUtil<f32>::inv_sqrt(sq), *reinterpret_cast<const TVec3f*>(&quat));
    }
    f32 angle = 2.0f * JGeometry::TUtil<f32>::acos(quat.w);
    if (__fabsf(angle) < 0.0872664675116539f &&
        PSVECDistance(&trans2, &trans1) < 10.0f) {
        mBounceCount++;
    } else {
        mBounceCount = 0;
    }

    MtxPtr pMtx2 = getBaseMtx();
    JMath::gekko_ps_copy12(&mMtxC4, pMtx2);
    if (mBounceCount >= 10) {
        return true;
    }
    return false;
}

// NONMATCHING: calls match; float expression scheduling differs.
void DiceBase::receiveMsgPushImpl(HitSensor* pSender, HitSensor* pReceiver, f32 scale) {
    TVec3f dir = pReceiver->mPosition - pSender->mPosition;
    f32 len = PSVECMag(&dir);
    if (MR::isNearZero(len, 0.001f)) {
        MtxPtr m1 = getBaseMtx();
        MtxPtr m2 = getBaseMtx();
        MtxPtr m3 = getBaseMtx();
        dir.set<f32>(m3[0][1], m2[1][1], m1[2][1]);
    } else {
        dir.scale(1.0f / len);
    }

    f32 radius = pReceiver->mRadius;
    TVec3f offset(dir);
    offset.scale(radius);
    TVec3f pos = pReceiver->mPosition - offset;
    f32 depth = scale * (pReceiver->mRadius + pSender->mRadius - len);

    f32 sq = dir.squared();
    if (sq <= 0.000003814697265625f) {
    } else {
        dir.scale(JGeometry::TUtil<f32>::inv_sqrt(sq) * depth);
    }

    mMover->collide(&pos, &dir);
}

void DiceBase::startHitStop() {
    mStopTimer = 30;
    mLandTimer = 300;
    mHitCount++;
}

bool DiceBase::isPushableSensor(HitSensor* pSensor) {
    if (MR::isSensorPlayerOrRide(pSensor) && MR::isCurrentRushRockMario() && !isNerve(GET_NERVE(ItemDice, ItemDiceNrvLand))) {
        return false;
    }
    return true;
}
