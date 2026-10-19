#include "Game/Camera/CameraViewInterpolator.hpp"
#include "Game/AreaObj/AreaObj.hpp"
#include "Game/AreaObj/CameraRepulsiveArea.hpp"
#include "Game/Camera/CameraLocalUtil.hpp"
#include "Game/Camera/CameraTargetObj.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/TriangleFilter.hpp"

#include <cmath>

namespace MR {
    template <> f32 sqrt<f32>(f32) NO_INLINE;
}
namespace CameraInterpolationUtil {
    void getRotationAxisAndAngle(const TPos3f&, TVec3f*, f32*) NO_INLINE;
}

namespace {
    static const f32 sCollisionBallRadius = 10000.0f;
    static const f32 sCollisionThroughDistance = 3400.0f;
    static const f32 sNearZ = 100.0f;
    f32 sMaxSwitchAngle;
    s32 sMaxSwitchAngleTimer;
    f32 sMaxRotationDifference;
    s32 sMaxRotationDifferenceTimer;
};  // namespace

CameraViewInterpolator::CameraViewInterpolator()
    : mInterpolateTimer(), mInterpolateTime(), mIsAntiOscillation(true), mIsForceCameraChange(true), mIsInterpolationOff(),
      mTargetPosition(0.0f, 0.0f, 0.0f), mRate(0.7f), mIsCollisionOff(), mIsRepulsionOff(), mTargetFovy(45.0f), mFovy(45.0f),
      mIsInterpolatingNearlyEnd(true), mIsInterpolating(), mIsCorrectErpPositionOn(true), mIsTargetOffsetOn(true), mCalcState(), mTargetObj(),
      mBinder(new Binder(nullptr, &mPosition, &mGravity, 100.0f, 0.0f, 64)), mPosition(0.0f, 0.0f, 0.0f), mGravity(0.0f, -1.0f, 0.0f),
      mIsCollided(true), mLErpOffset(0.0f, 0.0f, 0.0f), _C0() {
    mTargetMtx.identity();
    mMtx.identity();
    mBinder->setTriangleFilter(MR::createTriangleFilterFunc(MR::isCameraCodeThrough));
}

void CameraViewInterpolator::updateCameraMtx(MtxPtr pMtx, const TVec3f& rPos, const CameraTargetObj* pCameraObj, f32 fovy) {
    TVec3f targetPos = mTargetPosition;
    TPos3f targetMtx = mTargetMtx;

    updateCalcState(pCameraObj);
    interpolateCameraSwitching(pMtx, rPos, fovy);
    updateRotationDifference(targetMtx);

    if (!mIsRepulsionOff && translateByRepulsion()) {
        lookAtCenter();
    }

    bool isUpdated = calcCollision(targetMtx);
    mIsCollisionOff = false;
    reduceOscillation();

    TPos3f inv;
    inv.invert(mMtx);
    MR::setCameraViewMtx(inv, mInterpolateTime > 0, isUpdated, mTargetPosition);
    MR::setFovy(mFovy);

    mIsInterpolationOff = false;
    checkNearlyEnd(pMtx);
    if (!isInterpolating()) {
        mIsInterpolating = false;
    }
}

void CameraViewInterpolator::setInterpolation(u32 time) {
    if (!mIsInterpolationOff) {
        mInterpolateTimer = 0;
        mInterpolateTime = time;

        if (time == 0) {
            mIsForceCameraChange = true;
        }

        if (time == 0) {
            mIsInterpolatingNearlyEnd = true;
        } else {
            mIsInterpolatingNearlyEnd = false;
        }
    }

    mIsInterpolating = false;
}

bool CameraViewInterpolator::isInterpolating() const {
    return mInterpolateTime != 0 && mInterpolateTime > mInterpolateTimer;
}

bool CameraViewInterpolator::isInterpolatingNearlyEnd() const {
    if (isInterpolating()) {
        return mIsInterpolatingNearlyEnd;
    } else {
        return true;
    }
}

void CameraViewInterpolator::lookAtCenter() {
    TVec3f pos;
    mTargetMtx.getTrans(pos);
    TVec3f front;
    mTargetMtx.getZDir(front);

    TVec3f targetDir = pos - mTargetPosition;
    if (MR::isNearZero(targetDir)) {
        return;
    }

    MR::normalize(&targetDir);

    TPos3f rot;
    rot.setRotate(front, targetDir);

    mTargetMtx.setTrans(TVec3f(0.0f, 0.0f, 0.0f));
    mTargetMtx.concat(rot, mTargetMtx);
    mTargetMtx.setTrans(pos);
}

bool CameraViewInterpolator::calcCollision(const TPos3f& rMtx) {
    if (mIsCollisionOff || mIsInterpolating) {
        return false;
    }

    TVec3f pos;
    rMtx.getTrans(pos);
    TVec3f targetPos;
    mTargetMtx.getTrans(targetPos);

    mPosition.set(pos);
    resetGravity();

    targetPos += mLErpOffset;

    TVec3f bindDir;
    if (!calcBinder(&bindDir, targetPos, pos)) {
        mIsCollided = true;
        return false;
    }

    mIsCollided = false;
    mLErpOffset.zero();

    bindDir *= 0.25f;
    TVec3f newPos = pos;
    newPos += bindDir;
    TVec3f front = newPos - mTargetPosition;
    if (MR::isNearZero(front)) {
        return false;
    }

    MR::normalize(&front);

    TVec3f prevUp;
    rMtx.getYDir(prevUp);
    TVec3f up;
    mTargetMtx.getYDir(up);

    if (MR::abs(up.dot(front)) < 0.98f) {
        TVec3f side = up.cross(front);
        TVec3f newUp = front.cross(side);
        if (!MR::isNearZero(newUp)) {
            MR::normalize(&newUp);
            mTargetMtx.setXDir(side);
            mTargetMtx.setYDir(newUp);
            mTargetMtx.setZDir(front);
            mTargetMtx.setTrans(newPos);
        } else {
            mTargetMtx.setTrans(newPos);
        }
    } else if (MR::abs(prevUp.dot(front)) < 0.98f) {
        TVec3f side = prevUp.cross(front);
        TVec3f newUp = front.cross(side);
        if (!MR::isNearZero(newUp)) {
            MR::normalize(&newUp);
            mTargetMtx.setXDir(side);
            mTargetMtx.setYDir(newUp);
            mTargetMtx.setZDir(front);
            mTargetMtx.setTrans(newPos);
        } else {
            mTargetMtx.setTrans(newPos);
        }
    } else {
        mTargetMtx.setTrans(newPos);
    }

    return true;
}

bool CameraViewInterpolator::calcBinder(TVec3f* pBindDir, const TVec3f& rTargetPos, const TVec3f& rPos) {
    TVec3f diff = rTargetPos - rPos;
    f32 dist = diff.length();
    if (dist >= ::sCollisionThroughDistance) {
        mBinder->clear();
        return false;
    }

    mPosition.set(rPos);
    TVec3f grav = TVec3f(0.0f, -1.0f, 0.0f);
    mGravity.set(grav);

    f32 nearPlaneHeight = MR::tanDegree(MR::getFovy() * 0.5f) * ::sNearZ;
    f32 nearPlaneWidth = nearPlaneHeight * 16.0f / 9.0f;

    mBinder->mRadius = MR::sqrt(::sCollisionBallRadius + nearPlaneHeight * nearPlaneHeight + nearPlaneWidth * nearPlaneWidth);
    *pBindDir = mBinder->bind(diff);

    if (mBinder->mPlaneNum == 0) {
        return false;
    }

    return true;
}

void CameraViewInterpolator::reduceOscillation() {
    if (!mIsAntiOscillation || mIsForceCameraChange) {
        mIsForceCameraChange = false;
        mMtx.set(mTargetMtx);
        mFovy = mTargetFovy;
        return;
    }

    TQuat4f targetRot;
    mTargetMtx.getQuat(targetRot);
    TVec3f targetPos;
    mTargetMtx.getTrans(targetPos);

    TQuat4f rot;
    mMtx.getQuat(rot);
    TVec3f pos;
    mMtx.getTrans(pos);

    f32 invRate = 1.0f - mRate;
    TQuat4f interpRot;
    rot.slerp(targetRot, invRate);
    rot.normalize();
    pos = pos * mRate + targetPos * invRate;

    mMtx.makeQuat(rot);
    mMtx.setTrans(pos);
    mFovy = mFovy * 0.9f + mTargetFovy * 0.1f;
}

// NONMATCHING: regalloc
void CameraViewInterpolator::updateCalcState(const CameraTargetObj* pTargetObj) {
    switch (mCalcState) {
    case CalcState_Ready: {
        if (pTargetObj == nullptr || mTargetObj != pTargetObj) {
            mCalcState = CalcState_Invalid;
            break;
        }

        if (mIsTargetOffsetOn) {
            mTargetPosition += pTargetObj->getLastMove();
        }
        if (mIsCorrectErpPositionOn) {
            TVec3f targetPos;
            mTargetMtx.getTrans(targetPos);
            targetPos += pTargetObj->getLastMove();
            mTargetMtx.setTrans(targetPos);
        }

        break;
    }

    default: {
        if (pTargetObj != nullptr && (mInterpolateTime == 0 || mInterpolateTimer >= mInterpolateTime)) {
            mCalcState = CalcState_Ready;
        }

        break;
    }
    }

    mTargetObj = pTargetObj;
    mIsCorrectErpPositionOn = true;
}

void CameraViewInterpolator::interpolateCameraSwitching(MtxPtr pMtx, const TVec3f& rPos, f32 fovy) {
    updateRotationState(pMtx);
    if (mInterpolateTime == 0) {
        mTargetPosition.set(rPos);
        mTargetMtx.set(pMtx);
        mTargetFovy = fovy;
        return;
    }

    TPos3f newMtx = pMtx;

    TQuat4f rot;
    mTargetMtx.getQuat(rot);
    TVec3f up;
    mTargetMtx.getZDir(up);
    TVec3f pos;
    mTargetMtx.getTrans(pos);

    TQuat4f newTargetRot;
    newMtx.getQuat(newTargetRot);
    TVec3f newTargetUp;
    newMtx.getZDir(newTargetUp);
    TVec3f newTargetPos;
    newMtx.getTrans(newTargetPos);

    f32 rate = static_cast< f32 >(mInterpolateTimer) / static_cast< f32 >(mInterpolateTime);
    rate *= rate;

    CameraLocalUtil::slerpCamera(&rot, rot, newTargetRot, rate, false);
    rot.normalize();

    TPos3f mtx;
    mtx.makeQuat(rot);
    f32 currTargetDist = (mTargetPosition - pos).length();
    f32 newTargetDist = (rPos - newTargetPos).length();
    f32 dist = currTargetDist + rate * (newTargetDist - currTargetDist);

    mTargetPosition += (rPos - mTargetPosition) * rate;

    TVec3f newUp;
    mtx.getZDir(newUp);

    pos = mTargetPosition + newUp * dist;
    mtx.setTrans(pos);

    mTargetFovy += rate * (fovy - mTargetFovy);

    mInterpolateTimer++;
    if (mInterpolateTimer > mInterpolateTime) {
        mInterpolateTimer = mInterpolateTime;
    }

    mTargetMtx.set(mtx);
}

void CameraViewInterpolator::checkNearlyEnd(MtxPtr pMtx) {
    TPos3f newMtx = pMtx;
    TVec3f pos;
    mTargetMtx.getTrans(pos);
    TVec3f newTargetPos;
    newMtx.getTrans(newTargetPos);

    if (pos.distance(newTargetPos) > 1.0f) {
        mIsInterpolatingNearlyEnd = false;
        return;
    }

    TPos3f mtxDiff;
    mtxDiff.set(mTargetMtx);
    mtxDiff.invert(mtxDiff);
    mtxDiff.concat(newMtx, mtxDiff);

    TVec3f rotAxis;
    f32 angle;
    CameraInterpolationUtil::getRotationAxisAndAngle(mtxDiff, &rotAxis, &angle);
    if (angle > 1.0f) {
        mIsInterpolatingNearlyEnd = false;
        return;
    }

    mIsInterpolatingNearlyEnd = true;
}

bool CameraViewInterpolator::translateByRepulsion() {
    TVec3f pos;
    mTargetMtx.getTrans(pos);

    CameraRepulsiveArea* pArea = reinterpret_cast< CameraRepulsiveArea* >(MR::getAreaObj("CameraRepulsiveArea", pos));
    if (pArea == nullptr) {
        return false;
    }

    TVec3f repulsion = pArea->getRepulsion(pos);
    mTargetMtx.setTrans(pos + repulsion);
    return true;
}

void CameraInterpolationUtil::getRotationAxisAndAngle(const TPos3f& mtx, TVec3f* axis, f32* angle) {
    TQuat4f quat;
    mtx.getQuat(quat);
    f32 squared = reinterpret_cast<const TVec3f*>(&quat)->squared();
    if (squared <= MR::epsilon()) {
        *angle = 0.0f;
        axis->zero();
    } else {
        axis->scale(JGeometry::TUtil<f32>::inv_sqrt(squared), *reinterpret_cast<const TVec3f*>(&quat));
        *angle = 2.0f * JGeometry::TUtil<f32>::acos(quat.w);
    }
}

void CameraViewInterpolator::updateRotationState(MtxPtr matrix) {
    TPos3f next(matrix);
    TPos3f diff = mTargetMtx;
    diff.invert(diff);
    diff.concat(next);
    TVec3f axis;
    f32 angle;
    CameraInterpolationUtil::getRotationAxisAndAngle(diff, &axis, &angle);
    angle = -3.1415927f + static_cast<f32>(fmod(6.2831855f + (angle - -3.1415927f), static_cast<f64>(6.2831855f)));
    angle = MR::abs(angle);
    _C0 = angle > 1.2217306f;
    if (sMaxSwitchAngleTimer > 0) {
        if (--sMaxSwitchAngleTimer == 0) {
            sMaxSwitchAngle = 0.0f;
        }
    }
    if (sMaxSwitchAngle < angle) {
        sMaxSwitchAngle = angle;
        sMaxSwitchAngleTimer = 90;
    }
}

void CameraViewInterpolator::updateRotationDifference(MtxPtr matrix) {
    TPos3f previous(matrix);
    TQuat4f previousQuat;
    previous.getQuat(previousQuat);
    TQuat4f current;
    mTargetMtx.getQuat(current);
    f32 squared = PSQUATDotProduct(&current, &current);
    if (squared <= MR::epsilon()) {
        current.set<f32>(0.0f, 0.0f, 0.0f, 1.0f);
    } else {
        current.x = -current.x / squared;
        current.y = -current.y / squared;
        current.z = -current.z / squared;
        current.w = current.w / squared;
    }
    PSQUATMultiply(&current, &previousQuat, &current);
    const TVec3f& vector = *reinterpret_cast<const TVec3f*>(&current);
    f32 angle = 2.0f * JMAAsinRadian(MR::sqrt(vector.squared()));
    _C1 = angle > 0.034906585f;
    if (sMaxRotationDifferenceTimer > 0) {
        if (--sMaxRotationDifferenceTimer == 0) {
            sMaxRotationDifference = 0.0f;
        }
    }
    if (sMaxRotationDifference < angle) {
        sMaxRotationDifference = angle;
        sMaxRotationDifferenceTimer = 90;
    }
}
