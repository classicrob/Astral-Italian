#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleAccessor.hpp"
#include "Game/Boss/KoopaBattleCamera.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStateCommon.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"

KoopaLastBattleCamera::KoopaLastBattleCamera(LiveActor* pHost)
    : mHost(pHost) {
    mQuat.x = 0.0f;
    mQuat.y = 0.0f;
    mQuat.z = 0.0f;
    mQuat.w = 1.0f;
    mPlayerPos.x = 0.0f;
    mPlayerPos.y = 0.0f;
    mPlayerPos.z = 0.0f;
    mHostPos.x = 0.0f;
    mHostPos.y = 0.0f;
    mHostPos.z = 0.0f;
    mIsActive = false;
    mHostPos.set< f32 >(pHost->mPosition);
}

// NONMATCHING: the target negates dir with the inline paired-single negateInternal (ps_neg) and saves f31 with psq; ours calls TVec3f::operator-() out of line
void KoopaLastBattleCamera::start(f32 frames) {
    mHostPos.set< f32 >(mHost->mPosition);
    mPlayerPos.set< f32 >(*MR::getPlayerPos());
    TVec3f dir = mHostPos - mPlayerPos;
    MR::normalizeOrZero(&dir);
    TVec3f up;
    reinterpret_cast< KoopaBattle* >(mHost)->mBattleQuat.getYDir(up);
    reinterpret_cast< KoopaBattle* >(mHost)->mBattleQuat.getZDir(dir);
    TVec3f front = -dir;
    MR::makeQuatFrontUp(&mQuat, front, up);
    MR::startActorCameraProgrammable(mHost, static_cast< s32 >(frames));
    mIsActive = true;
}

void KoopaLastBattleCamera::end() {
    MR::endActorCameraProgrammable(mHost, -1, true);
    mIsActive = false;
}

void KoopaLastBattleCamera::update() {
    if (mIsActive) {
        updateQuat();
        applyCamera();
    }
}

void KoopaLastBattleCamera::updateQuat() {
    mHostPos.set< f32 >(mHost->mPosition);
    mPlayerPos.set< f32 >(*MR::getPlayerPos());
    TVec3f dir = mHostPos - *MR::getPlayerPos();
    MR::normalizeOrZero(&dir);
    if (!MR::isNearZero(dir, 0.001f)) {
        TVec3f up;
        reinterpret_cast< KoopaBattle* >(mHost)->mBattleQuat.getYDir(up);
        TQuat4f target;
        MR::makeQuatFrontUp(&target, dir, up);
        mQuat.slerp(target, 0.1f);
        MR::rotateQuatXAxisDegree(&mQuat, mQuat, 0.6f);
    }
}

// NONMATCHING: stack-slot permutation of the TVec3f temporaries only
inline TVec3f calcKoopaLastCameraBack(const TVec3f& front) {
    TVec3f back(front);
    back *= -1800.0f;
    return back;
}

void KoopaLastBattleCamera::applyCamera() {
    TVec3f front;
    mQuat.getZDir(front);
    TVec3f up;
    mQuat.getYDir(up);
    TVec3f offset(0.0f, 1.0f, 0.0f);
    offset *= 150.0f;
    TVec3f target;
    target.set< f32 >(mPlayerPos + offset);
    TVec3f camPos;
    camPos.set< f32 >(target + calcKoopaLastCameraBack(front));
    MR::setProgrammableCameraParam(mHost, target, camPos, up);
}
