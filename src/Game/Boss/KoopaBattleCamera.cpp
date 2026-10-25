#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleAccessor.hpp"
#include "Game/Boss/KoopaBattleCamera.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/Boss/KoopaStateCommon.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"

KoopaBattleCamera::KoopaBattleCamera(LiveActor* pHost)
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
    _44 = 0.0f;
    mIsActive = false;
    mHostPos.set< f32 >(pHost->mPosition);
}

void KoopaBattleCamera::startFacePlayer() {
    mHostPos.set< f32 >(mHost->mPosition);
    mPlayerPos.set< f32 >(*MR::getPlayerPos());
    TVec3f dir = mHostPos - mPlayerPos;
    MR::normalizeOrZero(&dir);
    TVec3f up;
    MR::calcUpVec(&up, mHost);
    MR::makeQuatFrontUp(&mQuat, dir, up);
    MR::startActorCameraProgrammable(mHost, 0);
    mIsActive = true;
    updateQuat();
    applyCamera();
    MR::resetCameraMan();
}

void KoopaBattleCamera::startFromCamera() {
    mHostPos.set< f32 >(mHost->mPosition);
    mPlayerPos.set< f32 >(*MR::getPlayerPos());
    MR::makeQuatFrontUp(&mQuat, MR::getCamZdir(), MR::getCamYdir());
    MR::startActorCameraProgrammable(mHost, -1);
    mIsActive = true;
    updateQuat();
    applyCamera();
}

void KoopaBattleCamera::end() {
    MR::endActorCameraProgrammable(mHost, -1, true);
    mIsActive = false;
}

void KoopaBattleCamera::update() {
    if (mIsActive) {
        updateQuat();
        applyCamera();
    }
}

void KoopaBattleCamera::updateQuat() {
    mHostPos.set< f32 >(mHost->mPosition);
    mPlayerPos.set< f32 >(*MR::getPlayerPos());
    TVec3f dir = mHostPos - *MR::getPlayerPos();
    MR::normalizeOrZero(&dir);
    TVec3f base;
    MR::getKoopaBattleBasePos(&base);
    TVec3f toPlayerDir;
    TVec3f toPlayer = *MR::getPlayerPos() - base;
    MR::normalizeOrZero(&toPlayerDir, toPlayer);
    f32 dist = PSVECDistance(&base, MR::getPlayerPos());
    f32 distRate = 1.0f - MR::normalize(dist - MR::getKoopaBattleRadius(), 0.0f, 1000.0f);
    if (!MR::isNearZero(dir, 0.001f)) {
        f32 dot = MR::normalize(toPlayerDir.dot(dir), -2.0f, 1.6f);
        f32 rate = MR::normalize(dot, 0.7f, 1.0f);
        f32 blend = rate + dot * (1.0f - rate);
        TVec3f negDir = -toPlayerDir;
        MR::vecBlendSphere(&dir, dir, negDir, blend * distRate);
        MR::turnQuatZDirRate(&mQuat, mQuat, dir, 0.1f);
    }
}

// NONMATCHING: stack-slot permutation of the TVec3f temporaries only (171 words, same calls/constants)
namespace {
    inline void calcKoopaCameraPosition(TVec3f* camPos, const TVec3f& target, const TVec3f& baseDiff, const TVec3f& rHostPos, const TVec3f& negFront) {
    TVec3f back(negFront);
    TVec3f baseDir;
    back *= 2400.0f;
    MR::normalizeOrZero(&baseDir, baseDiff);
    f32 hostDot = baseDir.dot(rHostPos);
    f32 backDot = baseDir.dot(back);
    f32 t = ((1500.0f + hostDot) - baseDir.dot(target)) / backDot;
    if (0.0f <= t && t <= 1.0f) {
        camPos->set< f32 >(target + back * t);
    } else {
        camPos->set< f32 >(target + negFront * 2400.0f);
    }
    }
}

void KoopaBattleCamera::applyCamera() {
    TVec3f front;
    front.set< f32 >(2.0f * (mQuat.x * mQuat.z) + 2.0f * (mQuat.w * mQuat.y), 2.0f * (mQuat.y * mQuat.z) - 2.0f * (mQuat.w * mQuat.x),
                     1.0f - 2.0f * (mQuat.x * mQuat.x) - 2.0f * (mQuat.y * mQuat.y));
    TVec3f up;
    up.set< f32 >(2.0f * (mQuat.x * mQuat.y) - 2.0f * (mQuat.w * mQuat.z), 1.0f - 2.0f * (mQuat.x * mQuat.x) - 2.0f * (mQuat.z * mQuat.z),
                  2.0f * (mQuat.y * mQuat.z) + 2.0f * (mQuat.w * mQuat.x));
    TVec3f playerUp;
    playerUp.x = 0.0f;
    playerUp.y = 1.0f;
    playerUp.z = 0.0f;
    MR::getPlayerUpVec(&playerUp);
    MR::normalizeOrZero(&playerUp);
    playerUp.orthogonalize(front);
    playerUp *= 400.0f;
    TVec3f target;
    target.set< f32 >(mPlayerPos + playerUp);
    TVec3f base;
    TVec3f camPos;
    MR::getKoopaBattleBasePos(&base);
    const TVec3f& baseDiff = base - mHost->mPosition;
    const TVec3f& rHostPos = mHost->mPosition;
    TVec3f negFront = -front;
    calcKoopaCameraPosition(&camPos, target, baseDiff, rHostPos, negFront);
    MR::setProgrammableCameraParam(mHost, target, camPos, up);
}
