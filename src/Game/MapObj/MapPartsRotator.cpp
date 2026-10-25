#include "Game/MapObj/MapPartsRotator.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/MapObj/MapParts.hpp"
#include "Game/MapObj/MapPartsRailRotator.hpp"
#include "Game/Util.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MathUtil.hpp"

namespace NrvMapPartsRotator {
    NEW_NERVE(HostTypeNeverMove, MapPartsRotator, NeverMove);
    NEW_NERVE(HostTypeWait, MapPartsRotator, Wait);
    NEW_NERVE(HostTypeRotateStart, MapPartsRotator, RotateStart);
    NEW_NERVE(HostTypeRotate, MapPartsRotator, Rotate);
    NEW_NERVE(HostTypeStopAtEnd, MapPartsRotator, StopAtEnd);
};  // namespace NrvMapPartsRotator

MapPartsRotator::MapPartsRotator(LiveActor* pActor) : MapPartsRotatorBase(pActor, "自身回転") {
    _18 = 0.0f;
    mRotateAngle = 0.0f;
    mRotateTime = 0;
    mRotateStopTime = 0;
    mTargetAngle = 0.0f;
    mRotateSpeed = 0.0f;
    mAngle = 0.0f;
    mRotateAxis = AxisType_XAxis;
    mRotateAccelType = 0;
    mRotateType = 1;
    mSignMotionType = 0;
    _A0 = 0.0f;
    mIsOnReverse = false;
    mBaseHostMtx.identity();
    mRotateMtx.identity();
    updateBaseHostMtx();
    mRotateMtx.set(mBaseHostMtx);
}

void MapPartsRotator::init(const JMapInfoIter& rIter) {
    MapPartsFunction::init(rIter);
    MR::getMapPartsArgRotateAngle(&mRotateAngle, rIter);
    MR::getMapPartsArgRotateAxis(&mRotateAxis, rIter);
    MR::getMapPartsArgRotateAccelType(&mRotateAccelType, rIter);
    MR::getMapPartsArgRotateSpeed(&_18, rIter);
    if (mRotateAccelType == 2) {
        MR::getMapPartsArgRotateTime(&mRotateTime, rIter);
    }
    initRotateSpeed();
    MR::getMapPartsArgRotateStopTime(&mRotateStopTime, rIter);
    MR::getMapPartsArgRotateType(&mRotateType, rIter);
    MR::getMapPartsArgSignMotionType(&mSignMotionType, rIter);

    if (MR::isNearZero(_18)) {
        initNerve(GET_NERVE(MapPartsRotator, HostTypeNeverMove));
    } else {
        initNerve(GET_NERVE(MapPartsRotator, HostTypeWait));
    }

    f32 angle;

    if (isRotateClockwise()) {
        angle = mRotateAngle;
    } else {
        angle = -mRotateAngle;
    }

    mTargetAngle = angle;
}

void MapPartsRotator::start() {
    updateTargetAngle();
    updateRotateMtx((AxisType)mRotateAxis, mAngle);
    setNerve(GET_NERVE(MapPartsRotator, HostTypeRotate));
}

void MapPartsRotator::end() {
    setNerve(GET_NERVE(MapPartsRotator, HostTypeWait));
}

bool MapPartsRotator::isWorking() const {
    if (!isNerve(GET_NERVE(MapPartsRotator, HostTypeNeverMove))) {
        return !isNerve(GET_NERVE(MapPartsRotator, HostTypeWait));
    }

    return false;
}

void MapPartsRotator::startWithSignalMotion() {
    updateBaseHostMtx();
    updateTargetAngle();
    setNerve(GET_NERVE(MapPartsRotator, HostTypeRotateStart));
}

void MapPartsRotator::cancelSignalMotion() {
    mAngle = _A0;
    updateRotateMtx((AxisType)mRotateAxis, _A0);
    setNerve(GET_NERVE(MapPartsRotator, HostTypeWait));
}

// NONMATCHING: source/destination pointer registers differ in the inline rotation copy.
inline const TVec3f& getMapPartsHostRotation(const LiveActor& host) {
    return host.mRotation;
}

void MapPartsRotator::updateBaseHostMtx() {
    mBaseHostMtx.setEuler(getMapPartsHostRotation(*mHost) * (PI / 180.0f));
}

bool MapPartsRotator::isMoving() const {
    return isNerve(GET_NERVE(MapPartsRotator, HostTypeRotate));
}

inline f32 signedRotateSpeedSquared(f32 speedSquared, f32 direction) {
    return speedSquared * direction;
}

void MapPartsRotator::updateVelocity() {
    if (MR::isNearZero(mRotateAngle) || isAccelTypeConstantSpeed()) {
        if (mRotateAccelType != 2) mRotateSpeed = 0.01f * _18;
        else mRotateSpeed = _18;
        mIsOnReverse = false;
        return;
    }

    if (mRotateAccelType != 1) {
        return;
    }

    f32 speed = _18;
    if (mRotateAccelType != 2) speed *= 0.01f;
    f32 direction = MR::sign(speed);
    f32 rotateAccel = signedRotateSpeedSquared(speed * speed, direction) / mRotateAngle;
    bool invert;

    if (isRotateClockwise()) {
        invert = mTargetAngle - (mRotateAngle * 0.5f) <= mAngle;
    } else {
        invert = mAngle <= mTargetAngle + (mRotateAngle * 0.5f);
    }

    if (invert) {
        rotateAccel *= -1.0f;
    }

    f32 oldRotateSpeed = mRotateSpeed;
    mRotateSpeed += rotateAccel;

    if ((oldRotateSpeed >= 0.0f && mRotateSpeed < 0.0f) || (oldRotateSpeed < 0.0f && mRotateSpeed >= 0.0f)) {
        mIsOnReverse = true;
    } else {
        mIsOnReverse = false;
    }
}

void MapPartsRotator::updateAngle() {
    mAngle += mRotateSpeed * getSpeed();

    if (MR::isNearZero(mRotateAngle)) {
        MR::repeatDegree(&mAngle);
    }
}

void MapPartsRotator::updateTargetAngle() {
    if (MR::isNearZero(mRotateAngle)) {
        return;
    }

    if (isRotateClockwise()) {
        mTargetAngle = mAngle + mRotateAngle;
    } else {
        mTargetAngle = mAngle - mRotateAngle;
    }
}

void MapPartsRotator::restartAtEnd() {
    if (mRotateType != 0) {
        if (mRotateType == 1) {
            _18 *= -1.0f;
        }

        updateTargetAngle();

        if (MR::hasMapPartsMoveStartSignMotion(mSignMotionType)) {
            setNerve(GET_NERVE(MapPartsRotator, HostTypeRotateStart));
        } else {
            setNerve(GET_NERVE(MapPartsRotator, HostTypeRotate));
        }
    }
}

void MapPartsRotator::initRotateSpeed() {
    if (mRotateAccelType == 2) {
        _18 = mRotateAngle / mRotateTime;
    }

    if (mRotateAngle < 0.0f) {
        _18 = 0.0f;
    }
}

bool MapPartsRotator::isReachedTargetAngle() const {
    if (MR::isNearZero(mRotateAngle)) {
        return false;
    }

    if (isRotateClockwise()) {
        return mTargetAngle <= mAngle;
    } else {
        return mAngle <= mTargetAngle;
    }
}

void MapPartsRotator::updateRotateMtx(AxisType type, f32 angle) {
    TVec3f rotateAxisDir;
    calcRotateAxisDir(type, &rotateAxisDir);
    mRotateMtx.identity();
    mRotateMtx.makeRotateDegree(rotateAxisDir, angle);
    mRotateMtx.concat(mRotateMtx, mBaseHostMtx);
}

void MapPartsRotator::calcRotateAxisDir(AxisType type, TVec3f* pDir) const {
    switch (type) {
    case AxisType_XAxis:
        mBaseHostMtx.getXDir(*pDir);
        break;
    case AxisType_YAxis:
        mBaseHostMtx.getYDir(*pDir);
        break;
    case AxisType_ZAxis:
        mBaseHostMtx.getZDir(*pDir);
        break;
    }
}

void MapPartsRotator::exeNeverMove() {
}

void MapPartsRotator::exeWait() {
}

void MapPartsRotator::exeRotate() {
    updateVelocity();
    updateAngle();
    if (isFirstStep()) MR::startActionSoundAuto(mHost, "RotatorRotateStart", -1, -1, -1);
    f32 speed = __fabsf(mRotateSpeed);
    s32 soundParam = 1000.0f * speed;
    if (speed >= 0.05f) MR::startActionSoundAuto(mHost, "RotatorRotateLoop", soundParam, -1, -1);

    if (isAccelTypeConstantSpeed() && isReachedTargetAngle()) {
        mAngle = mTargetAngle;
        updateRotateMtx((AxisType)mRotateAxis, mAngle);

        if (mRotateStopTime > 0 || mRotateType == 0) {
            setNerve(GET_NERVE(MapPartsRotator, HostTypeStopAtEnd));
        } else {
            restartAtEnd();
        }

        return;
    }

    if (mRotateAccelType == 1 && MR::isNearZero(mRotateSpeed, 0.00001f)) {
        setNerve(GET_NERVE(MapPartsRotator, HostTypeStopAtEnd));
        return;
    }

    updateRotateMtx((AxisType)mRotateAxis, mAngle);
}

void MapPartsRotator::exeRotateStart() {
    if (isFirstStep()) {
        _A0 = mAngle;
        MR::startActionSoundAuto(mHost, "RotatorRotateSignStart", -1, -1, -1);
    }
    MR::startActionSoundAuto(mHost, "RotatorRotateSignLoop", -1, -1, -1);

    mAngle += 0.5f * (getStep() / 3 % 2 == 0 ? 1.0f : -1.0f);
    updateRotateMtx((AxisType)mRotateAxis, mAngle);

    if (isStep(MapParts::getMoveStartSignalTime())) {
        mAngle = _A0;
        updateRotateMtx((AxisType)mRotateAxis, mAngle);
        setNerve(GET_NERVE(MapPartsRotator, HostTypeRotate));
    }
}

#pragma dont_inline on
void MapPartsRotator::exeStopAtEnd() {
    if (isFirstStep()) MR::startActionSoundAuto(mHost, "RotatorStopAtEnd", -1, -1, -1);
    MR::startActionSoundAuto(mHost, "RotatorStopAtEndLoop", -1, -1, -1);
    if (getStep() >= mRotateStopTime) {
        restartAtEnd();
    }
}

#pragma dont_inline reset
