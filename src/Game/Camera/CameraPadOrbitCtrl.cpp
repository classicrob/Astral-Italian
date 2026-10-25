#define JGEOMETRY_VEC3_COPY_NOINLINE
#define JGEOMETRY_VEC3_SET_NOINLINE
#define JGEOMETRY_VEC3_NEGATE_OPERATOR_NOINLINE
#define JGEOMETRY_VEC3_MI_NOINLINE
#include "Game/Camera/CameraPadOrbitCtrl.hpp"
#include "Game/Camera/CameraLocalUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include <JSystem/JGeometry/TUtil.hpp>

namespace {
    inline f32 crossMagnitude(const TVec3f& a, const TVec3f& b) {
        TVec3f cross;
        PSVECCrossProduct(&a, &b, &cross);
        return PSVECMag(&cross);
    }
}

CameraPadOrbitCtrl::CameraPadOrbitCtrl() : mRotating(false), mAngle(0.0f) {}
void CameraPadOrbitCtrl::reset() {
    mRotating = false;
    mAngle = 0.0f;
    mTargetAngle = 0.0f;
}
void CameraPadOrbitCtrl::update(const TVec3f& up, const TVec3f& front, const TVec3f& watch, TVec3f* pos) {
    if (mRotating) updateRotation(up, front, watch, pos);
    else updateInput(up, front, watch, pos);
}
// NONMATCHING: quaternion math register/stack scheduling
void CameraPadOrbitCtrl::updateRotation(const TVec3f& up, const TVec3f&, const TVec3f& watch, TVec3f* pos) {
    f32 previous = mAngle;
    if (mTargetAngle > mAngle) {
        mAngle += 0.08f;
        if (mAngle >= mTargetAngle) { mAngle = mTargetAngle; mRotating = false; }
    } else {
        mAngle -= 0.08f;
        if (mAngle <= mTargetAngle) { mAngle = mTargetAngle; mRotating = false; }
    }
    TVec3f offset = *pos - watch;
    TQuat4f rotation;
    f32 halfAngle = (mAngle - previous) * 0.5f;
    rotation.toTvec()->scale(sin(halfAngle), up);
    rotation.w = cos(halfAngle);
    rotation.transform(offset);
    *pos = watch + offset;
}
void CameraPadOrbitCtrl::updateInput(const TVec3f& up, const TVec3f& front, const TVec3f& watch, TVec3f* pos) {
    f32 roundAngle = 0.7853981852531433f;
    if (CameraLocalUtil::testCameraPadTriggerReset(true)) {
        TVec3f planarFront;
        TVec3f planarOffset;
        TVec3f side;
        JMAVECScaleAdd(&up, &front, &planarFront, -up.dot(front));
        if (MR::normalizeOrZero(&planarFront)) return;
        TVec3f toWatch = watch - *pos;
        JMAVECScaleAdd(&up, &toWatch, &planarOffset, -up.dot(toWatch));
        if (MR::normalizeOrZero(&planarOffset)) return;
        f32 mag = crossMagnitude(planarFront, planarOffset);
        mTargetAngle = MR::abs(JMath::sAtanTable.atan2_(mag, planarFront.dot(planarOffset)));
        PSVECCrossProduct(&planarOffset, &planarFront, &side);
        if (side.dot(up) < 0.0f) mTargetAngle = -mTargetAngle;
        mAngle = 0.0f;
        mRotating = true;
    } else if (CameraLocalUtil::testCameraPadTriggerRoundLeft()) {
        TVec3f camUp = MR::getCamYdir();
        if (camUp.dot(up) >= 0.0f) {
            mTargetAngle = -roundAngle;
        } else {
            mTargetAngle = roundAngle;
        }
        mAngle = 0.0f;
        mRotating = true;
    } else if (CameraLocalUtil::testCameraPadTriggerRoundRight()) {
        TVec3f camUp = MR::getCamYdir();
        if (camUp.dot(up) >= 0.0f) {
            mTargetAngle = roundAngle;
        } else {
            mTargetAngle = -roundAngle;
        }
        mAngle = 0.0f;
        mRotating = true;
    }
}
