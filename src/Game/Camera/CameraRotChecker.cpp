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

// NONMATCHING: intrinsic estimate materialization and equivalent branch/register scheduling.
void CameraRotChecker::update() {
    TPos3f mtx;
    mtx.concat(MR::getCameraViewMtx(), mMtx);

    TQuat4f rotation;
    mtx.getQuat(rotation);
    f32 vectorLength = rotation.toTvec()->squared();
    if (vectorLength > 0.0f) {
        f32 inverse = __frsqrte(vectorLength);
        vectorLength = inverse * vectorLength;
        f32 correction = vectorLength * inverse;
        correction = -(correction - 3.0f);
        vectorLength = (correction * vectorLength) * 0.5f;
    }
    mIsRotatingHard = MR::abs(2.0f * JMAAsinRadian(vectorLength)) > mThreshold;

    mMtx.set(MR::getCameraInvViewMtx());
}
