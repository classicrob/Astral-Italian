#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Util/DrawUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/Functor.hpp"

VolumeDrawer::VolumeDrawer(const char* pName, const char* pArcName, f32 (*pMtx)[4], int drawType) : LiveActor(pName) {
    mBaseMtx = pMtx;
    mHost = nullptr;
    mColor.set(0xFF, 0xA9, 0x00, 0xFF);
    mDrawPasses = 1;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    mScaleRate = 1.0f;
    mScaleStep = 0.0f;
    mIsEnabled = true;
    if (drawType < 0) {
        drawType = 0x1D;
    }
    MR::FunctorV0F functor(&MR::setupShadowVolumeDraw);
    MR::registerPreDrawFunctionIfNotExist(functor, drawType);
    MR::initActor(this, pArcName, false);
    MR::connectToScene(this, 0x2A, 0xB, -1, drawType);
    MR::invalidateClipping(this);
}

void VolumeDrawer::setHost(const LiveActor* pHost) {
    mHost = const_cast<LiveActor*>(pHost);
}

void VolumeDrawer::setColor(const Color8& rColor) {
    mColor = rColor;
}

void VolumeDrawer::setDrawPasses(s32 passes) {
    mDrawPasses = passes;
}

void VolumeDrawer::setScale(const TVec3f& rScale) {
    mScale = rScale;
}

void VolumeDrawer::setScaleUpRate(s32 frames) {
    if (frames <= 0) {
        mScaleRate = 1.0f;
        mScaleStep = 0.0f;
        return;
    }
    mScaleStep = 1.0f / (f32)frames;
}

void VolumeDrawer::setScaleDownRate(s32 frames) {
    if (frames <= 0) {
        mScaleRate = 0.0f;
        mScaleStep = 0.0f;
        return;
    }
    mScaleStep = -1.0f / (f32)frames;
}

void VolumeDrawer::control() {
    f32 rate = mScaleRate + mScaleStep;
    mScaleRate = rate;
    f32 clamped = 0.0f;
    if (rate < clamped) {
    } else {
        clamped = 1.0f;
        if (rate > clamped) {
        } else {
            clamped = rate;
        }
    }
    mScaleRate = clamped;
    if (isShowVolume()) {
        MR::showModel(this);
        return;
    }
    MR::hideModel(this);
}

bool VolumeDrawer::isShowVolume() const {
    if (mHost != nullptr && (MR::isDead(mHost) || MR::isClipped(mHost) || MR::isHiddenModel(mHost))) {
        return false;
    }
    if (mScaleRate == 0.0f) {
        return false;
    }
    return true;
}

void VolumeDrawer::calcAndSetBaseMtx() {
    if (mBaseMtx != nullptr) {
        mPosition.set(mBaseMtx[0][3], mBaseMtx[1][3], mBaseMtx[2][3]);
        MR::setBaseTRMtx(this, mBaseMtx);
        return;
    }
    LiveActor::calcAndSetBaseMtx();
}

namespace MR {
    VolumeDrawer* createVolumeDrawer(const char* pName, const char* pArcName, f32 (*pMtx)[4]) {
        VolumeDrawer* pDrawer = new VolumeDrawer(pName, pArcName, pMtx, 0x1C);
        pDrawer->initWithoutIter();
        return pDrawer;
    }
}  // namespace MR
