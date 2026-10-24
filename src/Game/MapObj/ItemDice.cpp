#define JGEOMETRY_OUTLINE_MATRIX_POSITION
#define JGEOMETRY_VEC3_WMR_OUTLINE
#define JGEOMETRY_VEC3_WMR_INLINE_OPS
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

#include "Game/MapObj/ItemDice.hpp"
#include "Game/Enemy/Kuribo.hpp"
#include "Game/MapObj/BenefitItemObj.hpp"
#pragma dont_inline on
#include "Game/MapObj/BenefitItemOneUp.hpp"
#pragma dont_inline off
#include <new>

// One center sphere and eight corner spheres. Constructors initialize the centers before play.
static DiceMoverPoint sDiceSpheres[9] = {
    {TVec3f(0.0f, 0.0f, 0.0f), 100.0f},
    {TVec3f(80.0f, 80.0f, 80.0f), 20.0f},
    {TVec3f(80.0f, 80.0f, -80.0f), 20.0f},
    {TVec3f(80.0f, -80.0f, 80.0f), 20.0f},
    {TVec3f(80.0f, -80.0f, -80.0f), 20.0f},
    {TVec3f(-80.0f, 80.0f, 80.0f), 20.0f},
    {TVec3f(-80.0f, 80.0f, -80.0f), 20.0f},
    {TVec3f(-80.0f, -80.0f, 80.0f), 20.0f},
    {TVec3f(-80.0f, -80.0f, -80.0f), 20.0f},
};

namespace ItemDiceFunction {
// NONMATCHING: direction extraction and sign branches compile differently.
s32 calcFaceIndex(MtxPtr source, const TVec3f* direction) {
    TRot3f matrix;
    matrix.set(source);
    TVec3f x;
    f32 xz = matrix.mMtx[2][0];
    f32 xy = matrix.mMtx[1][0];
    f32 xx = matrix.mMtx[0][0];
    x.set<f32>(xx, xy, xz);
    TVec3f y;
    matrix.getYDir(y);
    TVec3f z;
    matrix.getZDir(z);
    f32 dx = x.dot(*direction);
    f32 dy = y.dot(*direction);
    f32 dz = z.dot(*direction);
    s32 result;
    f32 ay = __fabsf(dy);
    f32 ax = __fabsf(dx);
    if (ax > ay) {
        if (ax > __fabsf(dz)) {
            if (dx < 0.0f) {
                result = 0;
            } else {
                result = 1;
            }
        } else {
            result = dz < 0.0f ? 4 : 5;
        }
    } else if (ay > __fabsf(dz)) {
        result = dy < 0.0f ? 2 : 3;
    } else {
        result = dz < 0.0f ? 4 : 5;
    }
    return result;
}
// NONMATCHING: matrix/vector helpers inline with the current shared headers.
void initItemPose(LiveActor* actor, const TVec3f* position, TRot3f* output, f32 angle) {
    TPos3f matrix;
    MR::setPosition(actor, *position);
    TVec3f gravity;
    MR::calcGravityVector(actor, &gravity, nullptr, 0);
    TVec3f front = *MR::getPlayerPos() - *position;
    MR::normalizeOrZero(&front);
    TVec3f right;
    PSVECCrossProduct(&front, &gravity, &right);
    if (!MR::normalizeOrZero(&right)) {
        TVec3f up;
        PSVECCrossProduct(&front, &right, &up);
        matrix.setXDir(right);
        matrix.setYDir(up);
        matrix.setZDir(front);
    }
    else {
        MR::makeMtxTR(matrix.toMtxPtr(), actor);
        TVec3f up;
        matrix.getYDir(up);
        TQuat4f quat;
        quat.setRotate(up, -gravity);
        TRot3f rotation;
        rotation.setQuat(quat);
        matrix.concat(rotation, matrix);
    }
    TPos3f rotation;
    rotation.makeRotate(TVec3f(0.0f, 1.0f, 0.0f), angle);
    matrix.concat(matrix, rotation);
    matrix.setTrans(*position);
    if (output) output->set(matrix);
    matrix.getEulerXYZ(actor->mRotation);
    actor->mRotation.x *= 57.2957763671875f;
    actor->mRotation.y *= 57.2957763671875f;
    actor->mRotation.z *= 57.2957763671875f;
}
}

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

    s32 arg;
    MR::getJMapInfoArg1WithInit(rIter, &arg);
    if (arg != -1) {
        mThrowScaleA = (f32)arg / 10.0f;
    }

    MR::getJMapInfoArg2WithInit(rIter, &arg);
    if (arg != -1) {
        mThrowScaleB = (f32)arg / 10.0f;
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

    MR::initStarPointerTarget(this, 145.0f, TVec3f(0.0f, 0.0f));
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

    if (mMover->mVelocityChanged) {
        f32 d = mMover->mChangedSpeed - 0.699999988079071f;
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
#ifdef GALAXY_NATIVE
        const TVec3f velocity(mGravity);
        s32 result = ItemDiceFunction::calcFaceIndex(getBaseMtx(), &velocity);
#else
        s32 result = ItemDiceFunction::calcFaceIndex(getBaseMtx(), &TVec3f(mGravity));
#endif
        if (isGoodResult(result)) {
            MR::startActionSound(this, "FixedGood", -1, -1, -1);
        } else {
            MR::startActionSound(this, "FixedBad", -1, -1, -1);
        }
    }

    if (MR::isStep(this, 10)) {
#ifdef GALAXY_NATIVE
        const TVec3f velocity(mGravity);
        s32 result = ItemDiceFunction::calcFaceIndex(getBaseMtx(), &velocity);
#else
        s32 result = ItemDiceFunction::calcFaceIndex(getBaseMtx(), &TVec3f(mGravity));
#endif
        setResultFrame(result);
    }

    if (MR::isGreaterEqualStep(this, 40)) {
        MR::emitEffect(this, "Explosion");
        mIsLanded = true;

#ifdef GALAXY_NATIVE
        const TVec3f velocity(mGravity);
        s32 result = ItemDiceFunction::calcFaceIndex(getBaseMtx(), &velocity);
#else
        s32 result = ItemDiceFunction::calcFaceIndex(getBaseMtx(), &TVec3f(mGravity));
#endif
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
        TVec3f up;
        if (tryThrow()) {
            return false;
        }
        if (!isNerve(GET_NERVE(ItemDice, ItemDiceNrvThrow))) {
            setNerve(GET_NERVE(ItemDice, ItemDiceNrvThrow));
            MtxPtr pMtx = getBaseMtx();
            TPos3f mtx;
            JMath::gekko_ps_copy12(&mtx, pMtx);
            TVec3f pos = mPosition - mGravity.scaleInline(10.0f);
            mtx.setTrans(pos);
            mMover->setMtx(mtx.toMtxPtr());
            MR::stopSceneForDefaultHit(6);
            MR::tryRumblePadStrong(this, 0);
        }
        MR::tryHitReaction(this, "\222\274\220\332\203q\203b\203g");
        startHitStop();
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
    mMover->mRadius = 200.0f;
}

// NONMATCHING: calls match; field stores reload mMover in a different order than the target.
void DiceBase::createMover(HitSensor* pSensor) {
    DiceMover* pMover = new DiceMover(0, 0);
    mMover = pMover;
    mMover->mRadius = 200.0f;
    mMover->mMaxSpeed = 20.0f;
    mMover->mRestitution = 2.0f;
    mMover->mExceptSensor = pSensor;
    mMover->mCheckSweptCollision = true;

    MtxPtr pMtx = getBaseMtx();
    mMover->setMtx(pMtx);

    DiceMoverPointArray* pBlock = new DiceMoverPointArray;
    if (pBlock) {
        pBlock->mCount = 9;
        pBlock->mPoints = sDiceSpheres;
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
#ifdef GALAXY_NATIVE
        TVec3f tmp = axis.scaleInline(scaleA);
        mMover->collide(&proj, &tmp);
#else
        mMover->collide(&proj, &axis.scaleInline(scaleA));
#endif
    }

    TVec3f pos = rPos - mPosition;
    MR::normalizeOrZero(&pos);
    pos.scale(50.0f);
    pos.add(mPosition);
    f32 scaleB = mThrowScaleB;
#ifdef GALAXY_NATIVE
    TVec3f vel = (-mGravity).scaleInline(scaleB);
    mMover->collide(&pos, &vel);
#else
    mMover->collide(&pos, &(-mGravity).scaleInline(scaleB));
#endif
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

    f32 angle;
    if (sq <= 0.000003814697265625f) {
        axis.x = axis.y = axis.z = 0.0f;
        angle = 0.0f;
    } else {
        axis.scale(JGeometry::TUtil<f32>::inv_sqrt(sq), *reinterpret_cast<const TVec3f*>(&quat));
        angle = 2.0f * JGeometry::TUtil<f32>::acos(quat.w);
    }
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
    f32 depth;
    f32 radius;
    f32 len;

    TVec3f dir = pReceiver->mPosition - pSender->mPosition;
    len = PSVECMag(&dir);
    if (MR::isNearZero(len, 0.001f)) {
        MtxPtr m1 = getBaseMtx();
        MtxPtr m2 = getBaseMtx();
        MtxPtr m3 = getBaseMtx();
        dir.set<f32>(m3[0][1], m2[1][1], m1[2][1]);
    } else {
        dir.scale(1.0f / len);
    }

    radius = pReceiver->mRadius;
    TVec3f pos = pReceiver->mPosition - dir.scaleInline(radius);
    f32 senderRadius = pSender->mRadius;
    f32 receiverRadius = pReceiver->mRadius;
    depth = scale * (receiverRadius + senderRadius - len);
    dir.setLength(depth);

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

namespace {
    inline const char* diceName(const char* pLabel) {
        return pLabel;
    }

    // The 0x154-byte BenefitItemOneUp is built by calling the BenefitItemObj ctor and then storing the
    // BenefitItemOneUp vtable (the inline 2-arg ctor of BenefitItemOneUp reproduces that shape).
}  // namespace

// ItemDice --------------------------------------------------------------

ItemDice::ItemDice(const char* pName) : DiceBase(pName) {
    mMainItem = nullptr;
    mSubItem = nullptr;
}

ItemDice::~ItemDice() {
}

const char* ItemDice::getModelName() {
    return diceName("ItemDice");
}

void ItemDice::initItems(const JMapInfoIter&) {
    mMainItem = new Kuribo("クリボー");
    MR::setPosition(mMainItem, mPosition);
    mMainItem->initWithoutIter();
    mMainItem->makeActorDead();

    BenefitItemObj* pSub = new BenefitItemOneUp("１ＵＰキノコ");
    mSubItem = pSub;
    MR::setPosition(mSubItem, mPosition);
    mSubItem->initWithoutIter();
    mSubItem->makeActorDead();
    mSubItem->_DE = 2;

    MR::declareCoin(this, 10);
}

void ItemDice::setAnimFrame() {
    MR::startBtp(this, diceName("ItemDice"));
    MR::setBtpFrameAndStop(this, 0.0f);
}

void ItemDice::setResultFrame(u32 result) {
    switch (result) {
    case 0:
    case 1:
        MR::setBtpFrameAndStop(this, 2.0f);
        break;
    case 2:
    case 3:
        MR::setBtpFrameAndStop(this, 1.0f);
        break;
    case 4:
    case 5:
        MR::setBtpFrameAndStop(this, 3.0f);
        break;
    }
}

bool ItemDice::makeResult(u32 result) {
    switch (result) {
    case 0:
    case 1:
        MR::appearCoinPop(this, mPosition, 10);
        return true;
    case 2:
    case 3:
        popSubItem();
        return true;
    case 4:
    case 5:
        placeMainItem();
        break;
    }
    return false;
}

bool ItemDice::isGoodResult(u32 result) const {
    return result <= 3;
}

// unnamed fn_802B4B50: orient the main item by the dice rotation and place it
void ItemDice::placeMainItem() {
    TRot3f rot;
    ItemDiceFunction::initItemPose(mMainItem, &mPosition, &rot, 0.0f);
    TQuat4f quat;
    rot.getQuat(quat);
    mMainItem->appearKuriboFromBox(mPosition, quat);
}

// unnamed fn_802B4BB0: pop the sub item out of the dice with a random speed
void ItemDice::popSubItem() {
    ItemDiceFunction::initItemPose(mSubItem, &mPosition, nullptr, 3.1415927f * MR::getRandom(-1.0f, 1.0f));
    mSubItem->_DE = 2;
    mSubItem->appearPop();
}

// MultipleChoiceDice -------------------------------------------------------

const char* MultipleChoiceDice::getModelName() {
    return diceName("ItemDice");
}

void MultipleChoiceDice::initItems(const JMapInfoIter& rIter) {
    MR::initUseStageSwitchWriteA(this, rIter);
    MR::initUseStageSwitchWriteB(this, rIter);
}

void MultipleChoiceDice::setAnimFrame() {
    MR::startBtp(this, diceName("ItemDice"));
    MR::setBtpFrameAndStop(this, 4.0f);
}

void MultipleChoiceDice::setResultFrame(u32 result) {
    switch (result) {
    case 0:
    case 1:
        MR::setBtpFrameAndStop(this, 6.0f);
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        MR::setBtpFrameAndStop(this, 5.0f);
        break;
    }
}

bool MultipleChoiceDice::makeResult(u32 result) {
    switch (result) {
    case 0:
    case 1:
        if (MR::isValidSwitchB(this)) {
            MR::onSwitchB(this);
        }
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
        return true;
    }
    return false;
}

bool MultipleChoiceDice::isGoodResult(u32 result) const {
    return result - 2 <= 3;
}

// PositiveDice -------------------------------------------------------------

PositiveDice::PositiveDice(const char* pName) : DiceBase(pName) {
    mItem = nullptr;
}

const char* PositiveDice::getModelName() {
    return diceName("PositiveDice");
}

void PositiveDice::initItems(const JMapInfoIter&) {
    BenefitItemObj* pItem = new BenefitItemOneUp("１ＵＰキノコ");
    mItem = pItem;
    MR::setPosition(mItem, mPosition);
    mItem->initWithoutIter();
    mItem->makeActorDead();
    mItem->_DE = 2;
    MR::declareStarPiece(this, 6);
}

void PositiveDice::setAnimFrame() {
    MR::startBtp(this, diceName("PositiveDice"));
    MR::setBtpFrameAndStop(this, 0.0f);
}

void PositiveDice::setResultFrame(u32 result) {
    if (result <= 1) goto item;
    if (result - 2 <= 1) goto piece;
    if ((s32)result == 4) goto item;
    if ((s32)result == 5) goto piece;
    return;
item:
    MR::setBtpFrameAndStop(this, 1.0f);
    return;
piece:
    MR::setBtpFrameAndStop(this, 2.0f);
}

bool PositiveDice::makeResult(u32 result) {
    if (result <= 1) goto item;
    if (result - 2 <= 1) goto piece;
    if ((s32)result == 4) goto item;
    if ((s32)result == 5) goto piece;
    goto failed;
item:
    popItem();
    return true;
piece:
    MR::appearStarPiece(this, mPosition, 1, 10.0f, 40.0f, false);
    return true;
failed:
    return false;
}

bool PositiveDice::isGoodResult(u32 result) const {
    return result <= 5;
}

// unnamed fn_802B4F90
void PositiveDice::popItem() {
    ItemDiceFunction::initItemPose(mItem, &mPosition, nullptr, 3.1415927f * MR::getRandom(-1.0f, 1.0f));
    mItem->_DE = 2;
    mItem->appearPop();
}

// LuckyDice ----------------------------------------------------------------

LuckyDice::LuckyDice(const char* pName) : DiceBase(pName) {
}

const char* LuckyDice::getModelName() {
    return diceName("LuckyDice");
}

void LuckyDice::initItems(const JMapInfoIter&) {
    for (u32 i = 0; i < 5; i++) {
        BenefitItemObj* pItem = new BenefitItemOneUp("１ＵＰキノコ");
        mItems[i] = pItem;
        MR::setPosition(mItems[i], mPosition);
        mItems[i]->initWithoutIter();
        mItems[i]->_DE = 2;
        mItems[i]->makeActorDead();
    }
    MR::declareStarPiece(this, 1000);
}

void LuckyDice::setAnimFrame() {
    MR::startBtp(this, diceName("LuckyDice"));
    MR::setBtpFrameAndStop(this, 0.0f);
}

void LuckyDice::setResultFrame(u32 result) {
    if (result <= 1) goto one;
    if (result - 4 <= 1) goto piece;
    if ((s32)result == 2) goto five;
    if ((s32)result == 3) goto three;
    return;
one:
    MR::setBtpFrameAndStop(this, 1.0f);
    return;
five:
    MR::setBtpFrameAndStop(this, 3.0f);
    return;
three:
    MR::setBtpFrameAndStop(this, 2.0f);
    return;
piece:
    MR::setBtpFrameAndStop(this, 4.0f);
}

bool LuckyDice::makeResult(u32 result) {
    if (result <= 1) goto one;
    if (result - 4 <= 1) goto piece;
    if ((s32)result == 2) goto five;
    if ((s32)result == 3) goto three;
    goto failed;
one:
    shootItems(1);
    return true;
five:
    shootItems(5);
    return true;
three:
    shootItems(3);
    return true;
piece:
    MR::appearStarPiece(this, mPosition, 1, 10.0f, 40.0f, false);
    return true;
failed:
    return false;
}

bool LuckyDice::isGoodResult(u32 result) const {
    return result <= 5;
}

// unnamed fn_802B5240 (0x268 bytes): spread `count` benefit items around the dice toward the player.
// NONMATCHING: empty SDK default constructors remain outlined; vector temporary stack placement differs.
void LuckyDice::shootItems(s32 count) {
    TVec3f toPlayer = *MR::getPlayerPos() - mPosition;
    MR::vecKillElement(toPlayer, mGravity, &toPlayer);
    if (MR::normalizeOrZero(&toPlayer)) {
        toPlayer.set<f32>(0.0f, 0.0f, 1.0f);
        TQuat4f q;
        q.setRotate(TVec3f(0.0f, -1.0f, 0.0f), mGravity);
        q.transform(toPlayer);
    }

    TVec3f axis;
#ifdef GALAXY_NATIVE
    TVec3f negGravity = -mGravity;
    PSVECCrossProduct(&negGravity, &toPlayer, &axis);
#else
    PSVECCrossProduct(&-mGravity, &toPlayer, &axis);
#endif
    MR::normalizeOrZero(&axis);

    TRot3f rot;
    rot.setXDir(axis);
    rot.setYDir(-mGravity);
    rot.setZDir(toPlayer);
    reinterpret_cast<TPos3f&>(rot).setTrans(TVec3f(0.0f, 0.0f, 0.0f));

    TPos3f spin;
    spin.makeRotate(TVec3f(0.0f, 1.0f, 0.0f), 6.2831855f / (f32)count);

    TVec3f yDir;
    rot.getYDir(yDir);
    for (s32 i = 0; i < count; i++) {
        TVec3f ang;
        rot.getEulerXYZ(ang);
        ang.scale(57.295776f);
        MR::setRotation(mItems[i], ang);

        TVec3f zDir;
        rot.getZDir(zDir);
        MR::setPosition(mItems[i], mPosition + zDir.scaleInline(80.0f));
        mItems[i]->appearThrow(zDir.scaleInline(10.0f) + yDir.scaleInline(20.0f));

        if (i != 4) {
            rot.concat(spin);
        } else {
            break;
        }
    }
}

MultipleChoiceDice::~MultipleChoiceDice() {
}

PositiveDice::~PositiveDice() {
}

LuckyDice::~LuckyDice() {
}
