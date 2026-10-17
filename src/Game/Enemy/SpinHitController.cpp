#include "Game/Enemy/SpinHitController.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

SpinHitController::SpinHitController(const LiveActor* pActor) {
    setParameter(pActor, 15, 3, 0.0f, 40.0f, 20.0f);
}

SpinHitController::SpinHitController(const LiveActor* pActor, s32 killStep, s32 stopSceneFrame, f32 gravityAccel, f32 horizontalSpeed,
                                     f32 upSpeed) {
    setParameter(pActor, killStep, stopSceneFrame, gravityAccel, horizontalSpeed, upSpeed);
}

void SpinHitController::setParameter(const LiveActor* pActor, s32 killStep, s32 stopSceneFrame, f32 gravityAccel, f32 horizontalSpeed,
                                     f32 upSpeed) {
    mKillStep = killStep;
    mStopSceneFrame = stopSceneFrame;
    mGravityAccel = gravityAccel;
    mHorizontalSpeed = horizontalSpeed;
    mUpSpeed = upSpeed;
}

void SpinHitController::start(LiveActor* pActor, const TVec3f& a2, const TVec3f& a3) {
    TVec3f v9(a3);
    v9 -= a2;
    MR::vecKillElement(v9, pActor->mGravity, &v9);
    MR::normalizeOrZero(&v9);
    v9 *= mHorizontalSpeed;
    f32 val = mUpSpeed;
    TVec3f v8(pActor->mGravity);
    v8 *= val;
    v9 -= v8;
    pActor->mVelocity.x = v9.x;
    pActor->mVelocity.y = v9.y;
    pActor->mVelocity.z = v9.z;
    mIsStopScene = true;
}

void SpinHitController::startWithoutStopScene(LiveActor* pActor, const TVec3f& a2, const TVec3f& a3) {
    start(pActor, a2, a3);
    mIsStopScene = false;
}

bool SpinHitController::execute(LiveActor* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::invalidateClipping(pActor);

        if (mIsStopScene) {
            MR::stopSceneForDefaultHit(mStopSceneFrame);
        }
    }

    f32 val = mGravityAccel;
    TVec3f v12(pActor->mGravity);
    v12 *= val;
    pActor->mVelocity.add(v12);

    if ((MR::isBinded(pActor) && MR::isGreaterStep(pActor, 1)) || MR::isGreaterStep(pActor, mKillStep)) {
        MR::validateClipping(pActor);
        pActor->kill();
        return true;
    }

    return false;
}
