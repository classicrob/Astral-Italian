#include "Game/MapObj/ClipAreaShape.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/MtxUtil.hpp"

ClipAreaShape::ClipAreaShape(const char* pName) : mModelData() {
    mModelData = MR::getJ3DModelData(pName);
}

bool ClipAreaShape::isInArea(const TVec3f& rPosition, const TPos3f& rMtx, const TVec3f& rScale) const {
    if (MR::isNearZero(rScale.x) || (MR::isNearZero(rScale.y) || MR::isNearZero(rScale.z))) {
        return false;
    }

    TPos3f mtx;
    mtx.identity();
    mtx.invert(rMtx);
    TVec3f srcVec;
    mtx.mult(rPosition, srcVec);

    srcVec.x = (srcVec.x / rScale.x);
    srcVec.y = (srcVec.y / rScale.y);
    srcVec.z = (srcVec.z / rScale.z);
    return isInArea(srcVec);
}

bool ClipAreaShape::isInArea(const TVec3f&) const {
    return false;
}

void ClipAreaShape::calcVolumeMatrix(TPos3f* pVolMtx, const TPos3f& rSrcMtx, const TVec3f& rScale) const {
    pVolMtx->set(rSrcMtx);
    MR::preScaleMtx(pVolMtx->toMtxPtr(), rScale);
}

void ClipAreaShape::drawVolumeShape(const TPos3f& rMtx, const TVec3f& rPos) const {
    TPos3f volMtx;
    volMtx.identity();
    calcVolumeMatrix(&volMtx, rMtx, rPos);
    MR::multMtx(volMtx, volMtx, MR::getCameraViewMtx());
    GXLoadPosMtxImm(volMtx, 0);
    MR::drawSimpleModel(mModelData);
}

ClipAreaShapeSphere::ClipAreaShapeSphere(const char* pName) : ClipAreaShape(pName), mRadius(500.0f) {
}

bool ClipAreaShapeSphere::isInArea(const TVec3f& rVec) const {
    return rVec.squared() < mRadius * mRadius;
}

void ClipAreaShapeSphere::calcVolumeMatrix(TPos3f* pPos, const TPos3f& rPos, const TVec3f& rVec) const {
    pPos->set(rPos);

    TVec3f stack_14 = (TVec3f(rVec.scaleInline(mRadius)));
    stack_14.scale(0.0099999998f);
    MR::preScaleMtx(pPos->toMtxPtr(), stack_14);
}

ClipAreaShapeBox::ClipAreaShapeBox(const char* pName, s32 u1) : ClipAreaShape(pName) {
    mRadius = 500.0f;
    _C = u1;
}

bool ClipAreaShapeBox::isInArea(const TVec3f& rVec) const {
    switch (_C) {
    case 0:
        return (MR::isInRange(rVec.x, -mRadius, mRadius) && MR::isInRange(rVec.y, -mRadius, mRadius) && MR::isInRange(rVec.z, -mRadius, mRadius));
    case 1:
        return (MR::isInRange(rVec.x, -mRadius, mRadius) && MR::isInRange(rVec.y, 0.0f, 2.0f * mRadius) && MR::isInRange(rVec.z, -mRadius, mRadius));
    }

    return false;
}

void ClipAreaShapeBox::calcVolumeMatrix(TPos3f* pPos, const TPos3f& rPos, const TVec3f& rVec) const {
    pPos->set(rPos);

    if (_C == 1) {
        MR::addTransMtxLocalY(pPos->toMtxPtr(), mRadius * rVec.y);
    }

    MR::preScaleMtx(pPos->toMtxPtr(), rVec);
}

ClipAreaShapeCylinder::ClipAreaShapeCylinder() : ClipAreaShape("VolumeCylinder") {
    mRadius = 500.0f;
    mHeight = 500.0f;
}

// NONMATCHING: branch layout only (target emits the in-range compare block out of line: bne over li r3,0 / b); logic matches.
bool ClipAreaShapeCylinder::isInArea(const TVec3f& rVec) const {
    f32 v = (rVec.y / mHeight);
    if (!MR::isInRange(v, 0.0f, 1.0f)) {
        return false;
    }

    return ((rVec.x * rVec.x) + (rVec.z * rVec.z)) < (mRadius * mRadius);
}

void ClipAreaShapeCylinder::calcBaseScale(TVec3f* pOut) const {
    pOut->set(mRadius / 500.0f, mHeight / 500.0f, mRadius / 500.0f);
}

static void mulScaleVec(const TVec3f& rBase, const TVec3f& rScale, TVec3f& rOut) NO_INLINE {
    JGeometry::mulInternal(&rBase.x, &rScale.x, &rOut.x);
}

void ClipAreaShapeCylinder::calcScale(TVec3f* pOut, const TVec3f& rScale) const {
    TVec3f base;
    calcBaseScale(&base);
    mulScaleVec(base, rScale, *pOut);
}

void ClipAreaShapeCylinder::calcVolumeMatrix(TPos3f* pVolMtx, const TPos3f& rSrcMtx, const TVec3f& rScale) const {
    pVolMtx->set(rSrcMtx);

    TVec3f scale;
    TVec3f base;
    calcBaseScale(&base);
    mulScaleVec(base, rScale, scale);
    MR::preScaleMtx(pVolMtx->toMtxPtr(), scale);
}

ClipAreaShapeCone::ClipAreaShapeCone(s32 a1) : ClipAreaShape("VolumeCone") {
    _8 = 500.0f;
    _C = 500.0f;
    _10 = a1;
}

bool ClipAreaShapeCone::isInArea(const TVec3f& rVec) const {
    f32 v3 = (rVec.y / _C);

    if (!MR::isInRange(v3, 0.0f, 1.0f)) {
        return false;
    }

    if (_10 == 1) {
        v3 = (1.0f - v3);
    }

    f32 v23 = ((rVec.x * rVec.x) + (rVec.z * rVec.z));
    f32 v24 = (v3 * _8) * (v3 * _8);
    return v23 < v24;
}
