#include "Game/Camera/CameraMtxRegParallel.hpp"
#include "Game/Camera/CamTranslatorMtxRegParallel.hpp"
#include "Game/Camera/CameraCalc.hpp"
#include "Game/Camera/CameraLocalUtil.hpp"
#include "Game/Camera/CameraTargetObj.hpp"

namespace {
    extern const f32 sHalfPi;
    extern const f32 sDefaultDistance;
    extern const f32 sMinimumDistance;
    extern const f32 sWatchPointBlendRate;
}

CameraMtxRegParallel::~CameraMtxRegParallel() {
}

CameraMtxRegParallel::CameraMtxRegParallel(const char* pName)
    : Camera(pName), mString(), mAngleX(), mAngleY(), mDist(sDefaultDistance), mWPoint(0.0f, 0.0f, 0.0f) {
}

void CameraMtxRegParallel::reset() {
    calcIdealPose();
}

CameraTargetObj* CameraMtxRegParallel::calc() {
    calcIdealPose();
    return CameraLocalUtil::getTarget(this);
}

CamTranslatorBase* CameraMtxRegParallel::createTranslator() {
    return new CamTranslatorMtxRegParallel(this);
}

void CameraMtxRegParallel::calcIdealPose() {
    if (mDist < sMinimumDistance) {
        mDist = sMinimumDistance;
    }

    TVec3f watchPoint;
    CameraLocalUtil::makeWatchPoint(&watchPoint, this, CameraLocalUtil::getTarget(this), sWatchPointBlendRate);

    TPos3f mtx;
    mtx.set(CameraLocalUtil::getMtxReg(mString));

    TVec3f pos;
    MR::polarToCross(TVec3f(0.0f, 0.0f, 0.0f), &pos, mDist, mAngleX, mAngleY - sHalfPi);

    mtx.mult33(pos);
    pos.add(watchPoint);

    TVec3f up;
    mtx.getYDir(up);

    TVec3f wPoint = mWPoint;
    mtx.mult33(wPoint);

    watchPoint.add(wPoint);
    CameraLocalUtil::setWatchPos(this, watchPoint);
    CameraLocalUtil::setPos(this, pos);
    CameraLocalUtil::setUpVec(this, up);
    CameraLocalUtil::setWatchUpVec(this, CameraLocalUtil::getTarget(this)->getUpVec());
}

namespace {
    const f32 sHalfPi = 1.5707964f;
    const f32 sDefaultDistance = 1000.0f;
    const f32 sMinimumDistance = 300.0f;
    const f32 sWatchPointBlendRate = 0.1f / 15.0f;
}
