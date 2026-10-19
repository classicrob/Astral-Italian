#include "Game/Camera/CameraRotChecker.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/MathUtil.hpp"

namespace {
    static const f32 sThreshold = 0.6f * JGeometry::TUtil< f32 >::PI() / 180.0f;
};  // namespace

CameraRotChecker::CameraRotChecker() {
    mMtx.identity();
    mThreshold = ::sThreshold;
}

// Keep the reciprocal estimate at FPR precision until the shared helper multiplies it.
void CameraRotChecker::update() {
    TPos3f mtx;
    mtx.concat(MR::getCameraViewMtx(), mMtx);

    TQuat4f rotation;
    mtx.getQuat(rotation);
    f32 vectorLength = MR::fastSqrtf(rotation.toTvec()->squared());
    mIsRotatingHard = MR::abs(2.0f * JMAAsinRadian(vectorLength)) > mThreshold;

    mMtx.set(MR::getCameraInvViewMtx());
}
