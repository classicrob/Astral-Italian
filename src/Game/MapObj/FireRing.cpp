#pragma auto_inline off
// NONMATCHING: calcBallPositions uses different vector inlining, stack and register scheduling.
// NONMATCHING: calcRadiusSpeed retains instruction/register scheduling differences.
// Symbol issue: target getBaseMtx__8FireRingFv lacks the const qualifier used by LiveActor's virtual.
#include "Game/MapObj/FireRing.hpp"
#include "Game/Util.hpp"
#include "Game/NameObj/NameObjExecuteHolder.hpp"

namespace NrvFireRing {
    NEW_NERVE(FireRingNrvWait, FireRing, Wait);
    NEW_NERVE(FireRingNrvAccel, FireRing, Accel);
    NEW_NERVE(FireRingNrvRotate, FireRing, Rotate);
};  // namespace NrvFireRing

FireRing::FireRing(const char* pName) : LiveActor(pName) {
    mBalls = nullptr;
    mBallCount = 8;
    mRotSpeed = 30.0f;
    mRadius = 300.0f;
    mRadiusMax = 300.0f;
    mRadiusMin = 300.0f;
    mRadiusSpeed = 0.0f;
    mDuration = 60;
    mIsShadowDropDirPtr = false;
    mShadowLength = 1000;
    mBaseMtx.identity();
}

void FireRing::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::makeMtxTR(mBaseMtx.toMtxPtr(), this);
    MR::connectToSceneMapObjMovement(this);
    initSound(3, "FireRing", nullptr, TVec3f(0.0f));
    MR::getJMapInfoArg0NoInit(rIter, &mRadiusMax);
    mRadiusMin = mRadiusMax;
    MR::getJMapInfoArg1NoInit(rIter, &mRotSpeed);
    MR::getJMapInfoArg2NoInit(rIter, &mBallCount);
    MR::getJMapInfoArg3NoInit(rIter, &mRadiusMin);
    MR::getJMapInfoArg4NoInit(rIter, &mDuration);
    MR::getJMapInfoArg5NoInit(rIter, &mIsShadowDropDirPtr);
    MR::getJMapInfoArg6NoInit(rIter, &mShadowLength);
    mRadius = mRadiusMin;
    MR::setGroupClipping(this, rIter, 32);
    MR::setClippingTypeSphere(this, mRadiusMax);
    MR::initUseStageSwitchReadA(this, rIter);
    MR::useStageSwitchAwake(this, rIter);
    if (MR::isValidSwitchA(this)) {
        mRadius = 100.0f;
        initNerve(&NrvFireRing::FireRingNrvWait::sInstance, 0);
    } else {
        initNerve(&NrvFireRing::FireRingNrvRotate::sInstance, 0);
    }
    initBalls(rIter);
    makeActorAppeared();
}

void FireRing::makeActorDead() {
    if (isNerve(&NrvFireRing::FireRingNrvRotate::sInstance) || isNerve(&NrvFireRing::FireRingNrvAccel::sInstance)) {
        for (s32 i = 0; i < mBallCount; i++) {
            MR::forceDeleteEffect(mBalls[i], "Move");
        }
    }
    LiveActor::makeActorDead();
}

void FireRing::startClipped() {
    LiveActor::startClipped();
    if (isNerve(&NrvFireRing::FireRingNrvRotate::sInstance) || isNerve(&NrvFireRing::FireRingNrvAccel::sInstance)) {
        for (s32 i = 0; i < mBallCount; i++) {
            MR::forceDeleteEffect(mBalls[i], "Move");
        }
    }
}

void FireRing::endClipped() {
    LiveActor::endClipped();
    for (s32 i = 0; i < mBallCount; i++) {
        if (!MR::isEffectValid(mBalls[i], "Move")) {
            MR::emitEffect(mBalls[i], "Move");
        }
    }
}

namespace {
    inline f32 calcScaledSensorRadius(f32 radius, f32 scale) {
        return (0.65f * radius) * scale;
    }
}

void FireRing::initBalls(const JMapInfoIter& rIter) {
    mBalls = new FireRingBall*[mBallCount];
    f32 zeroF = 0.0f;
    TVec3f zero(zeroF);
    for (s32 i = 0; i < mBallCount; i++) {
        mBalls[i] = new FireRingBall(this, "\x83\x74\x83\x40\x83\x43\x83\x41\x83\x8A\x83\x93\x83\x4F\x83\x7B\x81\x5B\x83\x8B", "FireRingBall");
    }
    calcBallPositions();
    for (s32 i = 0; i < mBallCount; i++) {
        mBalls[i]->initWithoutIter();
        mBalls[i]->setShadowDropLength((f32)mShadowLength);
        MR::startBtk(mBalls[i], "FireRingBall");
        MR::setBtkFrame(mBalls[i], zeroF);
        if (mIsShadowDropDirPtr) {
            mBalls[i]->setShadowDropDirectionPtr();
        }
        f32 scale = mScale.y;
        if (scale > 1.2f) {
            MR::setSensorRadius(mBalls[i], "Attack", calcScaledSensorRadius(MR::getSensorRadius(mBalls[i], "Attack"), scale));
        }
    }
}

#pragma push
#pragma opt_propagation off
void FireRing::calcBallPositions() {
    TVec3f up;
    TVec3f side;
    TVec3f front;
    MR::calcUpVec(&up, this);
    MR::calcSideVec(&side, this);
    MR::calcFrontVec(&front, this);

    f32 step = 360.0f / (f32)mBallCount;
    for (s32 i = 0; i < mBallCount; i++) {
        f32 angle = step * (f32)i;
        const f32 degreeToTableIndex = JMath::TSinCosTable<14, f32>::LEN / 360.0f;
        const s32 tableIndexMask = JMath::TSinCosTable<14, f32>::LEN - 1;
        f32 absAngle = angle;
        if (angle < 0.0f) {
            absAngle = -angle;
        }
        f32 cosVal = mRadius * JMath::sSinCosTable.table[(s32)(degreeToTableIndex * absAngle) & tableIndexMask].b1;
        f32 sinVal;
        if (angle < 0.0f) {
            sinVal = -JMath::sSinCosTable.table[(s32)(-degreeToTableIndex * angle) & tableIndexMask].a1;
        } else {
            sinVal = JMath::sSinCosTable.table[(s32)(degreeToTableIndex * angle) & tableIndexMask].a1;
        }

        TVec3f offset;
        offset = side * cosVal + up * (mRadius * sinVal);
        TVec3f ballPos = mPosition + offset;
        MR::setPosition(mBalls[i], ballPos);

        TVec3f dir = ballPos - mPosition;
        MR::normalizeOrZero(&dir, dir);
        TVec3f cross(0.0f);
        PSVECCrossProduct(&dir, &front, &cross);
        MR::normalizeOrZero(&cross, cross);
        mBalls[i]->setFrontAndUp(dir, cross);

        if (mIsShadowDropDirPtr) {
            TVec3f negDir = -dir;
            mBalls[i]->setShadowDropDirection(negDir);
        }
    }
}

#pragma pop

bool FireRing::isAnyBallBinding() const {
    for (s32 i = 0; i < mBallCount; i++) {
        if (mBalls[i]->isNerveBindStarPointer()) {
            return true;
        }
    }
    return false;
}

f32 FireRing::calcRadiusSpeed() const {
    return (mRadiusMax - mRadiusMin) / (f32)mDuration;
}

void FireRing::exeWait() {
    if (MR::isValidSwitchA(this) && MR::isOnSwitchA(this)) {
        s32 nDiv = 15;
        mRadiusSpeed = (mRadiusMax - 100.0f) / (f32)nDiv;
        setNerve(&NrvFireRing::FireRingNrvAccel::sInstance);
    }
}

void FireRing::exeAccel() {
    if (MR::isFirstStep(this)) {
        for (s32 i = 0; i < mBallCount; i++) {
            if (!MR::isEffectValid(mBalls[i], "Move")) {
                MR::emitEffect(mBalls[i], "Move");
            }
        }
    }
    mRadius += mRadiusSpeed;
    mRadius = MR::clamp(mRadius, 0.0f, mRadiusMax);
    if (!isAnyBallBinding()) {
        MR::rotateMtxLocalZDegree(mBaseMtx, mRotSpeed / 10.0f);
        calcBallPositions();
        if (MR::isNearZero(mRadius - mRadiusMax, 0.001f)) {
            mRadiusSpeed = calcRadiusSpeed();
            setNerve(&NrvFireRing::FireRingNrvRotate::sInstance);
        }
    }
}

void FireRing::exeRotate() {
    if (MR::isFirstStep(this)) {
        for (s32 i = 0; i < mBallCount; i++) {
            if (!MR::isEffectValid(mBalls[i], "Move")) {
                MR::emitEffect(mBalls[i], "Move");
            }
        }
    }
    if (!isAnyBallBinding()) {
        MR::startActionSound(this, "Move", -1, -1, -1);
        if (!MR::isNearZero(mRadiusMax - mRadiusMin, 0.001f)) {
            mRadius += mRadiusSpeed;
            mRadius = MR::clamp(mRadius, mRadiusMin, mRadiusMax);
            if (MR::isNearZero(mRadius - mRadiusMax, 0.001f)) {
                mRadiusSpeed = -calcRadiusSpeed();
            }
            if (MR::isNearZero(mRadius - mRadiusMin, 0.001f)) {
                mRadiusSpeed = calcRadiusSpeed();
            }
        }
        MR::rotateMtxLocalZDegree(mBaseMtx, mRotSpeed / 10.0f);
        calcBallPositions();
    }
}

MtxPtr FireRing::getBaseMtx() const {
    return const_cast< TPos3f* >(&mBaseMtx)->toMtxPtr();
}

FireRing::~FireRing() {}
