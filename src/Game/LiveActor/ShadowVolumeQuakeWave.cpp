#include "Game/LiveActor/ShadowVolumeQuakeWave.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"

ShadowVolumeQuakeWave::ShadowVolumeQuakeWave(LiveActor* pActor) : ShadowVolumeDrawer("影描画[クッパ地震衝撃波]") {
    mActor = pActor;
    mMtx = nullptr;
    _2C = 1000.0f;
    _30 = 0.0f;
    _34 = 2.5f;
}

void ShadowVolumeQuakeWave::loadModelDrawMtx() const {
    TPos3f mtx;
    mtx.identity();
    if (mMtx != nullptr) {
        mtx.set(mMtx);
    } else {
        mtx.set(mActor->getBaseMtx());
    }

    MR::preScaleMtx(mtx, TVec3f(_2C));
    PSMTXConcat(MR::getCameraViewMtx(), mtx, mtx);
    GXLoadPosMtxImm(mtx, 0);
    GXSetCurrentMtx(0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
}

void ShadowVolumeQuakeWave::drawShape() const {
    f32 cosA = MR::cosDegree(_30 - _34);
    f32 cosB = MR::cosDegree(_30 + _34);
    f32 sinA = MR::sinDegree(_30 - _34);
    f32 sinB = MR::sinDegree(_30 + _34);

    for (s32 i = 0; i < 0x20; i++) {
        f32 angle0 = (360.0f * (f32)i) / 32.0f;
        f32 angle1 = (360.0f * (f32)(i + 1)) / 32.0f;

        f32 cos0 = MR::cosDegree(angle0);
        f32 sin0 = MR::sinDegree(angle0);
        f32 cos1 = MR::cosDegree(angle1);
        f32 sin1 = MR::sinDegree(angle1);

        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 6);
        GXPosition3f32(0.0f, 0.0f, 0.0f);
        GXPosition3f32(sinA * sin1, cosA, sinA * cos1);
        GXPosition3f32(sinA * sin0, cosA, sinA * cos0);
        GXPosition3f32(sinB * sin1, cosB, sinB * cos1);
        GXPosition3f32(sinB * sin0, cosB, sinB * cos0);
        GXPosition3f32(0.0f, 0.0f, 0.0f);
    }
}

bool ShadowVolumeQuakeWave::isDraw() const {
    return MR::isValidDraw(mActor);
}

ShadowVolumeQuakeWave::~ShadowVolumeQuakeWave() {
}
