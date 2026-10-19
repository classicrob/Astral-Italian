#include "Game/MapObj/RotateHammerPressure.hpp"
#include "Game/Map/CollisionParts.hpp"
#include <revolution/mtx.h>
#include "Game/Util.hpp"

static const TVec3f sRotateHammerPressureOffsets[4] = {
    TVec3f(-800.0f, -600.0f, 0.0f),
    TVec3f(605.0f, 800.0f, 0.0f),
    TVec3f(1.0f, 1.0f, 1.0f),
    TVec3f(-600.0f, 450.0f, 0.0f),
};

RotateHammerPressure::RotateHammerPressure(const char* name)
    : RotateMoveObj(name), mUpDir(0.0f, 0.0f, 0.0f), mSideDir(0.0f, 0.0f, 0.0f), _1A8(false), _1A9(false), mPushCollision(nullptr) {
    PSMTXIdentity(mMatrices[0]);
    PSMTXIdentity(mMatrices[1]);
    PSMTXIdentity(mMatrices[2]);
    PSMTXIdentity(mMatrices[3]);
}

namespace JGeometry {
    template <>
    void TRotation3<TMatrix34<SMatrix34C<f32> > >::setScale(f32 scale) {
                this->mMtx[0][0] = scale;
                this->mMtx[0][1] = 0.0f;
                this->mMtx[0][2] = 0.0f;
                this->mMtx[1][0] = 0.0f;
                this->mMtx[1][1] = scale;
                this->mMtx[1][2] = 0.0f;
                this->mMtx[2][0] = 0.0f;
                this->mMtx[2][1] = 0.0f;
                this->mMtx[2][2] = scale;
    }
}

void RotateHammerPressure::init(const JMapInfoIter& rIter) {
    RotateMoveObj::init(rIter);
    MR::calcUpVec(&mUpDir, this);
    MR::calcSideVec(&mSideDir, this);
    s32 shadowLength = -1;
    MR::getJMapInfoArg2NoInit(rIter, &shadowLength);
    MR::setShadowDropDirection(this, "UpShadow", -mUpDir);
    MR::setShadowDropDirection(this, "SideShadow", mSideDir);
    MR::setShadowDropPositionMtxPtr(this, "UpShadow", mMatrices[1], TVec3f(0.0f, 0.0f, 0.0f));
    MR::setShadowDropPositionMtxPtr(this, "SideShadow", mMatrices[2], TVec3f(0.0f, 0.0f, 0.0f));
    MR::getJMapInfoArg3NoInit(rIter, &_1A9);
    mPushCollision = MR::createCollisionPartsFromLiveActor(this, "PushCollision", getSensor("Body"), static_cast<MR::CollisionScaleType>(2));
    MR::validateCollisionParts(mPushCollision);
    if (shadowLength != -1) {
        _1A8 = true;
        MR::invalidateShadow(this, "UpShadow");
        MR::invalidateShadow(this, "SideShadow");
    }
    PSMTXCopy(getBaseMtx(), mMatrices[0]);
    MR::setEffectHostMtx(this, "Stop", mMatrices[3]);
}

// NONMATCHING: vector stack slots, floating-point scheduling, and folded scale arithmetic differ.
void RotateHammerPressure::control() {
    const TVec3f* offsets = sRotateHammerPressureOffsets;
    MapObjActor::control();
    if (!_1A8) {
        TVec3f sideOffset;
        TVec3f upOffset;
        PSMTXMultVecSR(mMatrices[0], &offsets[1], &sideOffset);
        PSMTXMultVecSR(mMatrices[0], &offsets[0], &upOffset);
        MR::setShadowProjection(this, "UpShadow", mPosition + upOffset, -mUpDir, true);
        MR::setShadowProjection(this, "SideShadow", mPosition + sideOffset, mSideDir, true);
        TVec3f up;
        MR::calcUpVec(&up, this);
        f32 angle = 57.29578f * MR::diffAngleAbs(mUpDir, up);
        if (angle < 40.0f) {
            MR::invalidateShadow(this, "UpShadow");
        } else {
            mMatrices[3].set(getBaseMtx());
            TVec3f pos = mPosition + upOffset;
            MR::setMtxTrans(mMatrices[3], pos.x, pos.y, pos.z);
            MR::validateShadow(this, "UpShadow");
        }
        if (angle > 50.0f) {
            MR::invalidateShadow(this, "SideShadow");
        } else {
            mMatrices[3].set(getBaseMtx());
            TVec3f pos = mPosition + sideOffset;
            MR::setMtxTrans(mMatrices[3], pos.x, pos.y, pos.z);
            MR::validateShadow(this, "SideShadow");
        }
        f32 upScale = MR::clamp(0.5f + (angle - 40.0f) / (90.0f - 40.0f), 0.1f, 1.0f);
        mMatrices[1].set(getBaseMtx());
        mMatrices[1].setScale(upScale);
        f32 sideScale = 1.0f - MR::clamp(angle / 50.0f - 0.5f, 0.0f, 1.0f - 0.1f);
        mMatrices[2].set(getBaseMtx());
        mMatrices[2].setScale(sideScale);
    }
    TVec3f offset(offsets[3]);
    if (_1A9) offset.x = -offset.x;
    TVec3f up(0.0f);
    mMatrices[0].getZDir(up);
    if (_1A9) {
        TVec3f gravity(-mGravity);
        PSVECCrossProduct(&gravity, &up, &up);
    } else {
        PSVECCrossProduct(&mGravity, &up, &up);
    }
    TPos3f collisionMtx;
    MR::makeMtxUpFront(&collisionMtx, up, -mGravity);
    TPos3f mtx;
    mtx.set(getBaseMtx());
    MR::addTransMtxLocal(mtx, offset);
    TVec3f pos(0.0f);
    mtx.getTrans(pos);
    MR::setMtxTrans(collisionMtx, pos.x, pos.y, pos.z);
    mPushCollision->setMtx(collisionMtx);
}

RotateHammerPressure::~RotateHammerPressure() {
}
