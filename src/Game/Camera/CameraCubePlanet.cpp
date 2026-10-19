#define JGEOMETRY_VEC3_COPY_NOINLINE
#define JGEOMETRY_VEC3_SET_NOINLINE
#define JGEOMETRY_VEC3_NEGATE_OPERATOR_NOINLINE
#define JGEOMETRY_VEC3_MI_NOINLINE
#include "Game/Camera/CameraCubePlanet.hpp"
#include "Game/Camera/CamTranslatorCubePlanet.hpp"
#include "Game/Camera/CameraLocalUtil.hpp"
#include "Game/Camera/CameraTargetObj.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include <JSystem/JGeometry/TUtil.hpp>

CameraCubePlanet::CameraCubePlanet(const char* pName)
    : Camera(pName), mDist(3000.0f), mAngleX(MR::pi() / 6.0f), mAngleY(0.35f), mUp(0.0f, 1.0f, 0.0f), _70(new CameraPadOrbitCtrl()) {
}

void CameraCubePlanet::reset() {
    mUp.set(-CameraLocalUtil::getTarget(this)->getGravityVector());
    _70->reset();
}

// NONMATCHING: regalloc
CameraTargetObj* CameraCubePlanet::calc() {
    TVec3f pos = CameraLocalUtil::getPos(mCameraMan);
    TVec3f up = mUp;

    TQuat4f upRot;
    upRot.setRotate(mUp, -CameraLocalUtil::getTarget(this)->getGravityVector(), CameraLocalUtil::isForceCameraChange() ? 1.0f : mAngleY);
    upRot.transform(up);
    mUp.set(up);

    TVec3f watchPoint;
    CameraLocalUtil::makeWatchPointImm(&watchPoint, this, CameraLocalUtil::getTarget(this));
    CameraLocalUtil::setWatchPos(this, watchPoint);
    CameraLocalUtil::setWatchUpVec(this, up);

    TVec3f toWatch = watchPoint - pos;
    TVec3f front = toWatch - up * up.dot(toWatch);
    if (MR::isNearZero(front)) {
        return nullptr;
    }
    MR::normalize(&front);

    TVec3f side = up.cross(front);
    TQuat4f rot;
    f32 halfAngle = mAngleX * 0.5f;
    rot.toTvec()->scale(sin(halfAngle), side);
    rot.w = cos(halfAngle);
    rot.transform(up);
    rot.transform(front);
    front.setLength(mDist);
    TVec3f newPos = watchPoint - front;
    TVec3f sideDir = CameraLocalUtil::getTarget(this)->getSideVec().cross(up);
    if (MR::normalizeOrZero(&sideDir)) sideDir.set(CameraLocalUtil::getTarget(this)->getFrontVec());
    if (_60) _70->update(up, sideDir, watchPoint, &newPos);
    CameraLocalUtil::setPos(this, newPos);
    CameraLocalUtil::setUpVec(this, up);
    return CameraLocalUtil::getTarget(this);
}

CamTranslatorBase* CameraCubePlanet::createTranslator() {
    return new CamTranslatorCubePlanet(this);
}

bool CameraCubePlanet::isEnableToReset() const { return _60; }
bool CameraCubePlanet::isEnableToRoundRight() const { return _60; }
bool CameraCubePlanet::isEnableToRoundLeft() const { return _60; }

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
    TQuat4f rotated;
    rotated.x = rotation.y * offset.z - rotation.z * offset.y + rotation.w * offset.x;
    rotated.y = -rotation.x * offset.z + rotation.z * offset.x + rotation.w * offset.y;
    rotated.z = rotation.x * offset.y - rotation.y * offset.x + rotation.w * offset.z;
    rotated.w = -rotation.x * offset.x - rotation.y * offset.y - rotation.z * offset.z;
    offset.set<f32>(rotated.x * rotation.w + rotated.y * -rotation.z - rotated.z * -rotation.y + rotated.w * -rotation.x,
                    -rotated.x * -rotation.z + rotated.y * rotation.w + rotated.z * -rotation.x + rotated.w * -rotation.y,
                    rotated.x * -rotation.y - rotated.y * -rotation.x + rotated.z * rotation.w + rotated.w * -rotation.z);
    *pos = watch + offset;
}
// NONMATCHING: regalloc
void CameraPadOrbitCtrl::updateInput(const TVec3f& up, const TVec3f& front, const TVec3f& watch, TVec3f* pos) {
    f32 roundAngle = 0.7853981852531433f;
    if (CameraLocalUtil::testCameraPadTriggerReset(true)) {
        TVec3f planarFront;
        JMAVECScaleAdd(&up, &front, &planarFront, -up.dot(front));
        if (MR::normalizeOrZero(&planarFront)) return;
        TVec3f toWatch = watch - *pos;
        TVec3f planarOffset;
        JMAVECScaleAdd(&up, &toWatch, &planarOffset, -up.dot(toWatch));
        if (MR::normalizeOrZero(&planarOffset)) return;
        TVec3f cross;
        PSVECCrossProduct(&planarFront, &planarOffset, &cross);
        f32 mag = PSVECMag(&cross);
        mTargetAngle = MR::abs(JMath::sAtanTable.atan2_(mag, planarFront.dot(planarOffset)));
        TVec3f side;
        PSVECCrossProduct(&planarOffset, &planarFront, &side);
        if (side.dot(up) < 0.0f) mTargetAngle = -mTargetAngle;
        mAngle = 0.0f;
        mRotating = true;
    } else if (CameraLocalUtil::testCameraPadTriggerRoundLeft()) {
        TVec3f camUp = MR::getCamYdir();
        mTargetAngle = camUp.dot(up) >= 0.0f ? -roundAngle : roundAngle;
        mAngle = 0.0f;
        mRotating = true;
    } else if (CameraLocalUtil::testCameraPadTriggerRoundRight()) {
        TVec3f camUp = MR::getCamYdir();
        mTargetAngle = camUp.dot(up) >= 0.0f ? roundAngle : -roundAngle;
        mAngle = 0.0f;
        mRotating = true;
    }
}
