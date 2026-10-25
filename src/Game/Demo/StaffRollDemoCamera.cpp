#include "Game/Demo/StaffRollDemoCamera.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/Util.hpp"

// NONMATCHING: (remaining diffs are stack-slot order of the up/target/pos temporaries, sdata2 literal pool order and
// sda21 symbol names of the camera-name/constant objects; logic is complete)
namespace {
    const char* const sOpeningCameraName = "オープニング";
    const char* const sEndingCameraName = "エンディング";
    extern const f32 sTurnAroundSpeed;
    extern const f32 sMaxSpeed;
}  // namespace

namespace {
    inline TVec3f getStaffCameraUp(const StaffRollDemoCamera& camera) {
        return TVec3f(camera.mUp);
    }

    inline TVec3f getStaffCameraOffset(const StaffRollDemoCamera& camera) {
        TVec3f dir(camera.mDirection);
        dir.x *= 100.0f;
        dir.y *= 100.0f;
        dir.z *= 100.0f;
        return dir;
    }

    inline TVec3f getStaffCameraTarget(const StaffRollDemoCamera& camera) {
        return TVec3f(camera.mTarget);
    }
}

StaffRollDemoCamera::StaffRollDemoCamera(LiveActor* pHost, ActorCameraInfo* pCameraInfo, s32 mode)
    : mHost(pHost), mCameraInfo(pCameraInfo), mMode(mode), mFrame(0.0f) {
    mDirection.set(0.0f, 0.0f, 1.0f);
    mTarget.set(0.0f, 0.0f, 0.0f);
    mUp.set(0.0f, 1.0f, 0.0f);
    mSide.set(1.0f, 0.0f, 0.0f);
    const char* pName = sOpeningCameraName;
    MR::declareEventCameraProgrammable(pName);
    MR::declareEventCameraProgrammable(sEndingCameraName);
    MR::setProgrammableCameraParam(pName, calcCameraPos(), getStaffCameraTarget(*this), getStaffCameraUp(*this), true);
    MR::startGlobalEventCameraNoTarget(pName, 0);
}

TVec3f StaffRollDemoCamera::calcCameraPos() const {
    return getStaffCameraTarget(*this) + getStaffCameraOffset(*this);
}

const char* StaffRollDemoCamera::getCameraName() const {
    return mMode == 0 ? sOpeningCameraName : sEndingCameraName;
}

void StaffRollDemoCamera::start() {
    if (mMode == 0) {
        startTurnAround();
    } else if (mMode == 1) {
        startFromCurrentCamera();
    }
}

void StaffRollDemoCamera::update(bool isSlowDown) {
    if (mMode == 0) {
        updateTurnAround();
    } else if (mMode == 1) {
        if (isSlowDown) {
            updateSlowDown();
        } else {
            updateSpeedUp();
        }
    }
}

void StaffRollDemoCamera::end() {
    MR::endEventCamera(mCameraInfo, getCameraName(), false, -1);
}

void StaffRollDemoCamera::startTurnAround() {
    mFrame = sTurnAroundSpeed;
    mSide.set(mUp);
    MR::rotateVecDegree(&mSide, mDirection, -30.0f);
    MR::setProgrammableCameraParam(getCameraName(), calcCameraPos(), getStaffCameraTarget(*this), getStaffCameraUp(*this), true);
    MR::setProgrammableCameraParamFovy(getCameraName(), 55.0f);
    MR::startGlobalEventCameraNoTarget(getCameraName(), 0);
}

void StaffRollDemoCamera::updateTurnAround() {
    MR::setProgrammableCameraParam(getCameraName(), calcCameraPos(), getStaffCameraTarget(*this), getStaffCameraUp(*this), true);
    MR::rotateVecRadian(&mDirection, mSide, -sTurnAroundSpeed);
}

void StaffRollDemoCamera::startFromCurrentCamera() {
    mFrame = 0.0f;
    mTarget.set(MR::getCamPos());
    mDirection.set(MR::getCamZdir());
    mUp.set(MR::getCamYdir());
    mSide.set(MR::getCamXdir());
    MR::startGlobalEventCameraNoTarget(getCameraName(), -1);
    MR::disableGlobalEventCameraCollision(getCameraName());
}

void StaffRollDemoCamera::updateSpeedUp() {
    mFrame += 0.00005f;
    mFrame = mFrame >= sMaxSpeed ? sMaxSpeed : mFrame;
    MR::setProgrammableCameraParam(getCameraName(), calcCameraPos(), getStaffCameraTarget(*this), getStaffCameraUp(*this), true);
    MR::rotateVecRadian(&mDirection, mSide, mFrame);
    MR::rotateVecRadian(&mUp, mSide, mFrame);
}

void StaffRollDemoCamera::updateSlowDown() {
    mFrame *= 0.97f;
    if (mFrame <= 0.05f * sMaxSpeed) {
        mFrame = 0.0f;
    }
    MR::setProgrammableCameraParam(getCameraName(), calcCameraPos(), getStaffCameraTarget(*this), getStaffCameraUp(*this), true);
    MR::rotateVecRadian(&mDirection, mSide, mFrame);
    MR::rotateVecRadian(&mUp, mSide, mFrame);
}

namespace {
    const f32 sTurnAroundSpeed = 0.001f;
    const f32 sMaxSpeed = 0.0045f;
}  // namespace
