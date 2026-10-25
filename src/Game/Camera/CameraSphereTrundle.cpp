#include "Game/Camera/CameraSphereTrundle.hpp"
#include "Game/Camera/CameraLocalUtil.hpp"
#include "Game/Camera/CameraTargetObj.hpp"
#include "Game/Util/MathUtil.hpp"
#include "math_types.hpp"
#include <JSystem/JGeometry/TQuat.hpp>

CameraSphereTrundle::CameraSphereTrundle(const char* pName) : Camera(pName) {
    mOffset.set(0.0f, 0.0f, 0.0f);
    mLocalPos.set(1.0f, 0.0f, 0.0f);
    mDist = 2000.0f;
    mAngle = 0.0f;
    mFront.set(0.0f, 0.0f, 0.0f);
    mSide.set(0.0f, 0.0f, 0.0f);
    mUp.set(0.0f, 0.0f, 0.0f);
}

void CameraSphereTrundle::reset() {
    TVec3f sp20(mOffset);
    mZoneMatrix.mult(sp20, sp20);

    TVec3f sp14(mLocalPos);
    mZoneMatrix.mult33(sp14);

    TVec3f sp8 = CameraLocalUtil::getTarget(this)->getPosition() - sp20;
    mFront = sp8;
    if (MR::normalizeOrZero(&mFront)) {
        mFront = CameraLocalUtil::getTarget(this)->getFrontVec();
    }

    PSVECCrossProduct(&sp14, &mFront, &mSide);
    if (MR::normalizeOrZero(&mSide)) {
        mSide = CameraLocalUtil::getTarget(this)->getSideVec();
    }

    PSVECCrossProduct(&mFront, &mSide, &mUp);
    if (MR::normalizeOrZero(&mUp)) {
        mUp = CameraLocalUtil::getTarget(this)->getUpVec();
    }
}

// NONMATCHING: equivalent temporary/copy layout and register scheduling.
inline TVec3f scaleSphereTrundleOffset(const TVec3f& direction, f32 distance) {
    TVec3f offset(direction);
    offset.scale(distance);
    return offset;
}

inline f32 calcSphereTrundleHalfAngle(f32 angle) {
    return 0.5f * angle;
}

CameraTargetObj* CameraSphereTrundle::calc() {
    TVec3f sp58;
    CameraLocalUtil::makeWatchPoint(&sp58, this, CameraLocalUtil::getTarget(this), 0.0066666668f);

    TVec3f sp4C(mOffset);
    mZoneMatrix.mult(sp4C, sp4C);

    TVec3f sp40 = CameraLocalUtil::getTarget(this)->getPosition() - sp4C;
    if (MR::normalizeOrZero(&sp40)) {
        return NULL;
    }

    TQuat4f sp30;
    sp30.setRotate(mFront, sp40);
    sp30.transform(mSide);
    sp30.transform(mUp);

    mFront.set(sp40);
    if (MR::normalizeOrZero(&mFront)) {
        return NULL;
    }

    if (MR::normalizeOrZero(&mSide)) {
        return NULL;
    }

    PSVECCrossProduct(&mFront, &mSide, &mUp);
    if (MR::normalizeOrZero(&mUp)) {
        return NULL;
    }

    PSVECCrossProduct(&mUp, &mFront, &mSide);

    f32 angle = calcSphereTrundleHalfAngle(mAngle);
    sp30.toTvec()->scale((f32)sin(angle), mSide);
    sp30.w = (f32)cos(angle);

    TVec3f sp20(mFront);
    sp30.transform(sp20);

    CameraLocalUtil::setWatchPos(this, sp58);
    CameraLocalUtil::setWatchUpVec(this, CameraLocalUtil::getTarget(this)->getUpVec());

    CameraLocalUtil::setPos(this, sp58 + scaleSphereTrundleOffset(sp20, mDist));
    CameraLocalUtil::setUpVec(this, mUp);

    return CameraLocalUtil::getTarget(this);
}

CamTranslatorBase* CameraSphereTrundle::createTranslator() {
    return new CamTranslatorSphereTrundle(this);
}

CameraSphereTrundle::~CameraSphereTrundle() {
}
