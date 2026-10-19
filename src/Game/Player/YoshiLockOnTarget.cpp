#include "Game/Player/YoshiLockOnTarget.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Screen/YoshiTargetMark.hpp"
#include "Game/Screen/StarPointerTarget.hpp"
#include "Game/Util.hpp"
#include "Game/Util/YoshiLockOnUtil.hpp"

#ifdef GALAXY_NATIVE
namespace YoshiLockOnParams {
#else
namespace {
#endif
    const char* sParamFile = "lockonparams";
    s32 sTimer = 30;
    f32 sDistance0 = 1250.0f;
    f32 sDistance1 = 1250.0f;
    f32 sDistance2 = 1250.0f;
    f32 sDistance4 = 1250.0f;
    f32 sDistance5 = 1250.0f;
    s32 sTargetMax = 3;
}

#ifdef GALAXY_NATIVE
using namespace YoshiLockOnParams;
#endif

f32 getYoshiLockOnDistance(s32 type) {
    if (static_cast<u32>(type - YoshiLockOnTargetType::Capture) <= 1) return sDistance2;
    switch (type) {
    case YoshiLockOnTargetType::Eat:
        return sDistance0;
    case YoshiLockOnTargetType::Lick:
        return sDistance1;
    case YoshiLockOnTargetType::Pull:
        return sDistance4;
    case YoshiLockOnTargetType::Projectile:
        return sDistance5;
    default:
        return 0.0f;
    }
}

bool YoshiLockOnTargetControl::isInvalid() const {
    if (!mTarget || !mTarget->mActor || MR::isDead(mTarget->mActor) || !mTarget->mEligibleThisFrame) return true;
    return mFramesUntilExpiry == 0;
}

bool YoshiLockOnTargetControl::isMarkDead() const {
    if (!mMark) return true;
    return MR::isDead(mMark);
}

void YoshiLockOnTargetControl::copyTargetInfo(YoshiLockOnTargetDesc* out) const {
    out->mActor = mTarget->mActor;
    out->mMtx = mTarget->mMtx;
    out->mOffset = mTarget->mOffset;
    out->mType = mType;
}

void YoshiLockOnTargetControl::appearMark() {
    if (mMark) mMark->appear();
}

void YoshiLockOnTargetControl::disappearMark() {
    if (mMark) mMark->disappear();
}

void YoshiLockOnTargetControl::update(const TVec3f& pos, f32 distance) {
    updateMarkPosition();
    if (mFramesUntilExpiry) --mFramesUntilExpiry;
    if (PSVECDistance(&mTarget->mActor->mPosition, &pos) > distance) mFramesUntilExpiry = 0;
}

void YoshiLockOnTargetControl::calcScreenPosition() {
    if (mMark) return;
    TVec3f pos, x, y, z;
    if (!mTarget->mMtx) {
        pos.x = mTarget->mActor->mPosition.x;
        pos.y = mTarget->mActor->mPosition.y;
        pos.z = mTarget->mActor->mPosition.z;
    } else pos.set<f32>(mTarget->mMtx[0][3], mTarget->mMtx[1][3], mTarget->mMtx[2][3]);
    if (mTarget->mMtx) {
        x.set<f32>(mTarget->mMtx[0][0], mTarget->mMtx[1][0], mTarget->mMtx[2][0]);
        y.set<f32>(mTarget->mMtx[0][1], mTarget->mMtx[1][1], mTarget->mMtx[2][1]);
        z.set<f32>(mTarget->mMtx[0][2], mTarget->mMtx[1][2], mTarget->mMtx[2][2]);
    } else if (mTarget->mActor->getBaseMtx()) {
        TPos3f mtx;
        mtx.set(mTarget->mActor->getBaseMtx());
        mtx.getXDir(x);
        mtx.getYDir(y);
        mtx.getZDir(z);
    } else {
        TPos3f mtx;
        MR::makeMtxTR(mtx.toMtxPtr(), mTarget->mActor);
        mtx.getXDir(x);
        mtx.getYDir(y);
        mtx.getZDir(z);
    }
    x.scale(mTarget->mOffset.x);
    y.scale(mTarget->mOffset.y);
    z.scale(mTarget->mOffset.z);
    pos.add(x + y + z);
    MR::calcScreenPosition(&pos, pos);
    pos.z = 0.0f;
}

void YoshiLockOnTargetControl::updateMarkPosition() {
    if (mMark) {
        TVec3f pos;
        mTarget->mActor->mStarPointerTarget->calcPosition(&pos);
        mMark->mTargetPosition = pos;
    }
}

void YoshiLockOnTargetControl::invalidate() {
    if (mTarget) mTarget->mEligibleThisFrame = false;
}

#ifndef GALAXY_NATIVE
#include "../src/Game/Player/YoshiLockOnManager.cpp"
#endif

// NONMATCHING: copy load/store scheduling and FPR allocation differ.
YoshiLockOnTarget::YoshiLockOnTarget(LiveActor* actor, MtxPtr mtx, const TVec3f& offset) : mActor(actor), mMtx(mtx) {
    JMathInlineVEC::PSVECCopy(&offset, &mOffset);
    mEligibleThisFrame = false;
}

namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor* actor, MtxPtr mtx, const TVec3f& offset) {
        return new YoshiLockOnTarget(actor, mtx, offset);
    }

    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor* actor, const char* jointName, const TVec3f& offset) {
        MtxPtr mtx = getJointMtx(actor, jointName);
        return new YoshiLockOnTarget(actor, mtx, offset);
    }
}
