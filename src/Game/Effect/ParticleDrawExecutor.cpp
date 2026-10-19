#include "Game/Effect/ParticleDrawExecutor.hpp"
#include "Game/Util.hpp"
#include "Game/Effect/EffectSystem.hpp"
#include "Game/NameObj/NameObjAdaptor.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SystemUtil.hpp"
#include <JSystem/J3DGraphBase/J3DSys.hpp>
#include <JSystem/JParticle/JPAEmitterManager.hpp>
#include <JSystem/JUtility/JUTVideo.hpp>

namespace {
    void connectToSceneDrawAdaptor(NameObjAdaptor* pAdaptor, const MR::FunctorBase& rFunctor, int drawType) NO_INLINE {
        pAdaptor->connectToDraw(rFunctor);
        MR::connectToScene(pAdaptor, -1, -1, -1, drawType);
        MR::registerPreDrawFunction(MR::FunctorV0F(&ParticleDrawExecutor::initDraw), drawType);
    }
};  // namespace

ParticleDrawExecutor::ParticleDrawExecutor(const EffectSystem* pSystem, bool createAdaptors)
    : mHost(pSystem), mDraw3DAdaptor(), mDraw2DAdaptor(), mDrawIndirectAdaptor(), mDrawAfterIndirectAdaptor(), mDraw2DModelAdaptor(),
      mDrawBloomAdaptor(), mDrawAfterImageEffectAdaptor(), mDrawWorldMapAdaptor(), mIsDrawEnabled(true), _21() {
    if (createAdaptors) {
        initDrawAdaptor();
    }
}

void ParticleDrawExecutor::initDraw() {
    GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, Color8(0, 0, 0, 0));
}

void ParticleDrawExecutor::draw3D() const {
    drawWithViewMtx3D(MR::getCameraViewMtx());
}

// NONMATCHING: FP/register scheduling
void ParticleDrawExecutor::draw2D() const {
    if (mIsDrawEnabled) {
        TPos3f cameraMtx;
        cameraMtx.identity();
        JPADrawInfo drawInfo(cameraMtx.toMtxPtr());

        f32 height = static_cast< s32 >(JUTVideo::getManager()->getEfbHeight());
        height *= 0.5f;
        f32 width = MR::getScreenWidth();
        width *= 0.5f;

        Mtx44 projection;
        C_MTXOrtho(projection, -height, height, -width, width, -10000.0f, 10000.0f);
        GXSetProjection(projection, GX_ORTHOGRAPHIC);
        GXSetCullMode(GX_CULL_NONE);
        GXSetZMode(GX_FALSE, GX_NEVER, GX_FALSE);
        C_MTXLightOrtho(drawInfo.mPrjMtx, height, -height, -width, width, 0.5f, 0.5f, 0.5f, 0.5f);
        mHost->mEmitterManager->draw(&drawInfo, 6);
        GXSetClipMode(GX_CLIP_ENABLE);
        mHost->mEmitterManager->draw(&drawInfo, 7);
        GXSetClipMode(GX_CLIP_ENABLE);
    }
}

void ParticleDrawExecutor::drawIndirect() const {
    if (mIsDrawEnabled) {
        JPADrawInfo drawInfo(j3dSys.getViewMtx(), MR::getFovy(), MR::getAspect());
        mHost->mEmitterManager->draw(&drawInfo, 2);
        GXSetClipMode(GX_CLIP_ENABLE);
    }
}

void ParticleDrawExecutor::drawAfterIndirect() const {
    drawWithViewMtxAfterIndirect(MR::getCameraViewMtx());
}

void ParticleDrawExecutor::drawFor2DModel() const {
    if (mIsDrawEnabled) {
        JPADrawInfo drawInfo(j3dSys.getViewMtx());
        mHost->mEmitterManager->draw(&drawInfo, 8);
        GXSetClipMode(GX_CLIP_ENABLE);
    }
}

void ParticleDrawExecutor::drawForBloomEffect() const {
    drawWithViewMtxForBloomEffect(MR::getCameraViewMtx());
}

void ParticleDrawExecutor::drawAfterImageEffect() const {
    drawWithViewMtxAfterImageEffect(MR::getCameraViewMtx());
}

void ParticleDrawExecutor::drawForWorldMap() const {
    drawWithViewMtxForWorldMap(MR::getCameraViewMtx());
}

void ParticleDrawExecutor::drawWithViewMtx3D(const TPos3f& rViewMtx) const {
    if (mIsDrawEnabled) {
        JPADrawInfo drawInfo(rViewMtx);
        mHost->mEmitterManager->draw(&drawInfo, 0);
        GXSetClipMode(GX_CLIP_ENABLE);
        mHost->mEmitterManager->draw(&drawInfo, 1);
        GXSetClipMode(GX_CLIP_ENABLE);
    }
}

void ParticleDrawExecutor::drawWithViewMtxAfterIndirect(const TPos3f& rViewMtx) const {
    if (mIsDrawEnabled) {
        JPADrawInfo drawInfo(rViewMtx);
        mHost->mEmitterManager->draw(&drawInfo, 3);
        GXSetClipMode(GX_CLIP_ENABLE);
    }
}

void ParticleDrawExecutor::drawWithViewMtxForBloomEffect(const TPos3f& rViewMtx) const {
    if (mIsDrawEnabled) {
        JPADrawInfo drawInfo(rViewMtx);
        mHost->mEmitterManager->draw(&drawInfo, 4);
        GXSetClipMode(GX_CLIP_ENABLE);
    }
}

void ParticleDrawExecutor::drawWithViewMtxAfterImageEffect(const TPos3f& rViewMtx) const {
    if (mIsDrawEnabled) {
        JPADrawInfo drawInfo(rViewMtx);
        mHost->mEmitterManager->draw(&drawInfo, 5);
        GXSetClipMode(GX_CLIP_ENABLE);
    }
}

void ParticleDrawExecutor::drawWithViewMtxForWorldMap(const TPos3f& view) const {
    if (mIsDrawEnabled) {
        JPADrawInfo info(view);
        mHost->mEmitterManager->draw(&info, 9);
        GXSetClipMode(GX_CLIP_ENABLE);
    }
}

void ParticleDrawExecutor::initDrawAdaptor() {
    mDraw3DAdaptor = new NameObjAdaptor("3Dパーティクル");
    ::connectToSceneDrawAdaptor(mDraw3DAdaptor, MR::Functor(this, &ParticleDrawExecutor::draw3D), MR::DrawType_EffectDraw3D);

    mDraw2DAdaptor = new NameObjAdaptor("2Dパーティクル");
    ::connectToSceneDrawAdaptor(mDraw2DAdaptor, MR::Functor(this, &ParticleDrawExecutor::draw2D), MR::DrawType_EffectDraw2D);

    mDrawIndirectAdaptor = new NameObjAdaptor("インダイレクトパーティクル");
    ::connectToSceneDrawAdaptor(mDrawIndirectAdaptor, MR::Functor(this, &ParticleDrawExecutor::drawIndirect), MR::DrawType_EffectDrawIndirect);

    mDrawAfterIndirectAdaptor = new NameObjAdaptor("インダイレクト後パーティクル");
    ::connectToSceneDrawAdaptor(mDrawAfterIndirectAdaptor, MR::Functor(this, &ParticleDrawExecutor::drawAfterIndirect), MR::DrawType_EffectDrawAfterIndirect);

    mDraw2DModelAdaptor = new NameObjAdaptor("2Dモデル用パーティクル");
    ::connectToSceneDrawAdaptor(mDraw2DModelAdaptor, MR::Functor(this, &ParticleDrawExecutor::drawFor2DModel), MR::DrawType_EffectDrawFor2DModel);

    mDrawBloomAdaptor = new NameObjAdaptor("ブルーム用パーティクル");
    ::connectToSceneDrawAdaptor(mDrawBloomAdaptor, MR::Functor(this, &ParticleDrawExecutor::drawForBloomEffect), MR::DrawType_EffectDrawForBloomEffect);

    mDrawAfterImageEffectAdaptor = new NameObjAdaptor("イメージエフェクト後パーティクル");
    ::connectToSceneDrawAdaptor(mDrawAfterImageEffectAdaptor, MR::Functor(this, &ParticleDrawExecutor::drawAfterImageEffect), MR::DrawType_EffectDrawAfterImageEffect);

    mDrawWorldMapAdaptor = new NameObjAdaptor("ワールドマップアイコン用パーティクル");
    ::connectToSceneDrawAdaptor(mDrawWorldMapAdaptor, MR::Functor(this, &ParticleDrawExecutor::drawForWorldMap), MR::DrawType_EffectDrawWorldMapMiniIcon);
}

void MR::FunctorV0F::operator()() const {
    reinterpret_cast<void (*)()>(mFunc)();
}

MR::FunctorBase* MR::FunctorV0F::clone(JKRHeap* heap) const {
    return new (heap, 0) FunctorV0F(*this);
}
