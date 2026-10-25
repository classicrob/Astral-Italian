#define JGEOMETRY_VEC3_SET_NOINLINE
#pragma auto_inline off
#include "Game/Gravity/BarrelGravity.hpp"
#include "Game/Util/MathUtil.hpp"

void BarrelGravity::updateMtx(const TPos3f& rMtx) {
    rMtx.mult(mLocalPosition, mPosition);

    f32 ux = mLocalUpDirection.x;
    f32 uy = mLocalUpDirection.y;
    f32 uz = mLocalUpDirection.z;
    mUpDirection.set<f32>(uz * rMtx.mMtx[0][2] + (ux * rMtx.mMtx[0][0] + uy * rMtx.mMtx[0][1]),
                     uz * rMtx.mMtx[1][2] + (ux * rMtx.mMtx[1][0] + uy * rMtx.mMtx[1][1]),
                     uz * rMtx.mMtx[2][2] + (ux * rMtx.mMtx[2][0] + uy * rMtx.mMtx[2][1]));

    f32 scalar;
    MR::separateScalarAndDirection(&scalar, &mUpDirection, mUpDirection);
    mScaledRadius = mRadius * scalar;
}

bool BarrelGravity::calcOwnGravityVector(TVec3f* pDest, f32* pScalar, const TVec3f& rPosition) const {
    TVec3f sp54;
    sp54 = rPosition - mPosition;

    f32 d = sp54.dot(mUpDirection);
    if (d <= 0.0f || d >= mScaledRadius) {
        return false;
    }

    TVec3f sp48 = mPosition + mUpDirection * d;
    f32 dist;
    TVec3f sp3C;
    TVec3f sp30;
    MR::separateScalarAndDirection(&dist, &sp3C, sp48 - rPosition);
    if (MR::isNearZero(sp3C, 0.001f)) {
        MR::makeAxisVerticalZX(&sp30, mUpDirection);
        return true;
    }

    PSVECCrossProduct(&sp3C, &mUpDirection, &sp30);

    f32 angle = MR::clamp(mInwardsThreshold / dist, 0.0f, 0.7853982f);
    MR::rotateVecRadian(&sp30, mUpDirection, angle);
    MR::normalize(&sp30);

    if (!isInRangeDistance(dist)) {
        return false;
    }

    *pScalar = dist;
    pDest->set(sp30);
    return true;
}
