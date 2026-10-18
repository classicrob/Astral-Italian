#pragma auto_inline off
// NONMATCHING (not yet byte-exact, logic written from asm):
//   calcBallPositions (fn_802A25F0): 0x2CC vs 0x2BC, register/order diffs in the sin/cos table math.
//   exeAccel / exeRotate: 4-16 byte size diffs, same calls; regalloc in the clamp/rotate sequence.
//   initBalls (fn_802A2420): 93.7%, regalloc only (target keeps the 0.0f zero in f28 for setBtkFrame).
//   getBaseMtx: target symbol is non-const (getBaseMtx__8FireRingFv), LiveActor::getBaseMtx is const.
//   __sinit_FireRing_cpp: 97.7%, nerve sbss order matches, ctor layout differs.
#include "Game/MapObj/FireRing.hpp"
#include "Game/Util.hpp"
#include "Game/NameObj/NameObjExecuteHolder.hpp"

// SMG2 LiveActor::mScale sits at 0x30 (target asm); the shared header still says 0x2C.
static inline f32 getScaleX(const LiveActor* pActor) {
    return *reinterpret_cast< const f32* >(reinterpret_cast< const u8* >(pActor) + 0x30);
}

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

void FireRing::initBalls(const JMapInfoIter& rIter) {
    mBalls = new FireRingBall*[mBallCount];
    TVec3f zero(0.0f);
    for (s32 i = 0; i < mBallCount; i++) {
        mBalls[i] = new FireRingBall(this, "\x83\x74\x83\x40\x83\x43\x83\x41\x83\x8A\x83\x93\x83\x4F\x83\x7B\x81\x5B\x83\x8B", "FireRingBall");
    }
    calcBallPositions();
    for (s32 i = 0; i < mBallCount; i++) {
        mBalls[i]->initWithoutIter();
        mBalls[i]->setShadowDropLength((f32)mShadowLength);
        MR::startBtk(mBalls[i], "FireRingBall");
        MR::setBtkFrame(mBalls[i], 0.0f);
        if (mIsShadowDropDirPtr) {
            mBalls[i]->setShadowDropDirectionPtr();
        }
        f32 scale = getScaleX(this);
        if (scale > 1.2f) {
            MR::setSensorRadius(mBalls[i], "Attack", 0.65f * MR::getSensorRadius(mBalls[i], "Attack") * scale);
        }
    }
}

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
        f32 absAngle = angle;
        if (angle < 0.0f) {
            absAngle = -angle;
        }
        f32 cosVal = mRadius * reinterpret_cast< const f32* >(reinterpret_cast< const u8* >(&JMath::sSinCosTable) + ((s32)(45.511112f * absAngle) * 8 & 0x1FFF8))[1];
        f32 sinVal;
        if (angle < 0.0f) {
            sinVal = -*reinterpret_cast< const f32* >(reinterpret_cast< const u8* >(&JMath::sSinCosTable) + ((s32)(-45.511112f * angle) * 8 & 0x1FFF8));
        } else {
            sinVal = *reinterpret_cast< const f32* >(reinterpret_cast< const u8* >(&JMath::sSinCosTable) + ((s32)(45.511112f * angle) * 8 & 0x1FFF8));
        }

        TVec3f upPart;
        upPart = up * (mRadius * sinVal);
        TVec3f sidePart;
        sidePart = side * cosVal;
        TVec3f offset = sidePart + upPart;
        TVec3f ballPos = mPosition + offset;
        MR::setPosition(mBalls[i], ballPos);

        TVec3f dir = ballPos - mPosition;
        MR::normalizeOrZero(&dir, dir);
        TVec3f cross(0.0f);
        PSVECCrossProduct(&dir, &front, &cross);
        MR::normalizeOrZero(&cross, cross);
        mBalls[i]->setFrontAndUp(dir, cross);

        if (mIsShadowDropDirPtr) {
            TVec3f negDir(-dir.x, -dir.y, -dir.z);
            mBalls[i]->setShadowDropDirection(negDir);
        }
    }
}

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
    mRadius = MR::clamp(mRadius + mRadiusSpeed, 0.0f, mRadiusMax);
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
            mRadius = MR::clamp(mRadius + mRadiusSpeed, mRadiusMin, mRadiusMax);
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
