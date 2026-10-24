#define JGEOMETRY_VEC3_COPY_NOINLINE
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Util/DrawUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/Functor.hpp"
#include "Game/Util/MathUtil.hpp"

VolumeDrawer::VolumeDrawer(const char* pName, const char* pArcName, f32 (*pMtx)[4], int drawType)
    : LiveActor(pName), mBaseMtx(pMtx), mHost(nullptr), mColor(0xFF, 0xA9, 0x00, 0xFF), mDrawPasses(1), mScale(0.05f, 0.05f, 0.05f),
      mScaleRate(1.0f), mScaleStep(0.0f), mIsEnabled(true) {
    if (drawType < 0) {
        drawType = 0x1D;
    }
    MR::FunctorV0F functor(&MR::setupShadowVolumeDraw);
    MR::registerPreDrawFunctionIfNotExist(functor, drawType);
    MR::initActor(this, pArcName, false);
    MR::connectToScene(this, 0x2A, 0xB, -1, drawType);
    MR::invalidateClipping(this);
}

void VolumeDrawer::init(const JMapInfoIter&) {
    makeActorAppeared();
}

void VolumeDrawer::setHost(const LiveActor* pHost) {
    mHost = const_cast<LiveActor*>(pHost);
}

void VolumeDrawer::setColor(const Color8& rColor) {
    mColor.set(rColor);
}

void VolumeDrawer::setDrawPasses(s32 passes) {
    mDrawPasses = passes;
}

void VolumeDrawer::setScale(const TVec3f& rScale) {
    mScale.x = rScale.x;
    mScale.y = rScale.y;
    mScale.z = rScale.z;
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

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void VolumeDrawer::preScaleDrawPass(s32 pass) const {
    TVec3f out;
    f32 mtx[3][4];
    PSMTXConcat(MR::getCameraViewMtx(), getBaseMtx(), mtx);

    TVec3f scale(mScale);
    scale.x *= (f32)pass;
    scale.y *= (f32)pass;
    scale.z *= (f32)pass;

    TVec3f one(1.0f);
    TVec3f diff(one);
    diff.sub(scale);
    out.mul(LiveActor::mScale, diff);

    MR::preScaleMtx(mtx, out);
    GXLoadPosMtxImm(mtx, 0);
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void VolumeDrawer::draw() const {
    if (mScaleRate <= 0.0f) {
        return;
    }

    MR::loadProjectionMtx();

    u8 alpha = (u8)(s32)((f32)mColor.a * mScaleRate);
    Color8 color(mColor.r, mColor.g, mColor.b, alpha);

    for (s32 i = 0; i < mDrawPasses; i++) {
        preScaleDrawPass(i);
        GXSetCurrentMtx(0);
        GXSetColorUpdate(0);
        GXSetAlphaUpdate(1);
        GXSetDstAlpha(1, 0);
        GXSetCullMode(GX_CULL_NONE);
        GXSetTevColor(GX_TEVREG0, color);
        if (i == 0) {
            GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        }

        GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_COPY);
        MR::drawSimpleModel(MR::getJ3DModelData(this));

        GXSetDstAlpha(0, 0);
        GXSetZMode(GX_TRUE, GX_GEQUAL, GX_FALSE);
        GXSetCullMode(GX_CULL_FRONT);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ZERO, GX_LO_COPY);
        MR::drawSimpleModel(MR::getJ3DModelData(this));

        GXSetCullMode(GX_CULL_BACK);
        GXSetBlendMode(GX_BM_SUBTRACT, GX_BL_ZERO, GX_BL_ZERO, GX_LO_COPY);
        MR::drawSimpleModel(MR::getJ3DModelData(this));

        GXSetTevColor(GX_TEVREG0, color);
        GXSetCullMode(GX_CULL_NONE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_DSTALPHA, GX_BL_ONE, GX_LO_COPY);
        GXSetColorUpdate(1);
        GXSetDstAlpha(1, 0);
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        MR::drawSimpleModel(MR::getJ3DModelData(this));
    }
}

void VolumeDrawer::control() {
    mScaleRate += mScaleStep;
    mScaleRate = MR::clamp(mScaleRate, 0.0f, 1.0f);
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
        mPosition.set<f32>(mBaseMtx[0][3], mBaseMtx[1][3], mBaseMtx[2][3]);
        MR::setBaseTRMtx(this, mBaseMtx);
    } else {
        LiveActor::calcAndSetBaseMtx();
    }
}

namespace MR {
    VolumeDrawer* createVolumeDrawer(const char* pName, const char* pArcName, f32 (*pMtx)[4]) {
        VolumeDrawer* pDrawer = new VolumeDrawer(pName, pArcName, pMtx, 0x1C);
        pDrawer->initWithoutIter();
        return pDrawer;
    }
}  // namespace MR
