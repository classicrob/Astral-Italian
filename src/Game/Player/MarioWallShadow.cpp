#define JGEOMETRY_VEC3_MUL_NOINLINE
#include "Game/Player/MarioWallShadow.hpp"
#include "Game/Util/DirectDraw.hpp"
#include "Game/Util/MathUtil.hpp"
#include <JSystem/JUtility/JUTTexture.hpp>
#include <revolution/gx.h>

MarioWallShadow::MarioWallShadow() : mActive(0) {
    mPosition.x = 0.0f;
    mPosition.y = 0.0f;
    mPosition.z = 0.0f;
    mNormal.x = 0.0f;
    mNormal.y = 0.0f;
    mNormal.z = 0.0f;
    mDistance = 1000.0f;
}

// NONMATCHING: vector temporary placement and loop FPR allocation differ.
void MarioWallShadow::draw(JUTTexture* texture) {
    if (!mActive) {
        return;
    }

    f32 angle;
    f32 radius = 100.0f;
    TDDraw::setup(1, 1, 0);
    GXSetZMode(GX_TRUE, GX_GREATER, GX_FALSE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    texture->load(GX_TEXMAP0);
    GXClearVtxDesc();
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    Mtx rotation;
    TVec3f tangent(0.0f, mNormal.z, -mNormal.y);
    if (MR::isNearZero(tangent, 0.001f)) {
        tangent.set<f32>(mNormal.z, 0.0f, mNormal.x);
    }
    MR::normalizeOrZero(&tangent);
    PSMTXRotAxisRad(rotation, &mNormal, 0.3926991f);
    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 18);
    GXPosition3f32(mPosition.x - 5.0f * mNormal.x, mPosition.y - 5.0f * mNormal.y, mPosition.z - 5.0f * mNormal.z);
    GXColor1u32(128);
    GXTexCoord2f32(0.0f, 0.0f);
    for (u32 i = 0; i <= 16; i++) {
        angle = 2.0f * ((i / 16.0f) * JGeometry::TUtil<f32>::PI());
        TVec3f point = (mPosition - mNormal * 5.0f) + tangent * radius;
        GXPosition3f32(point.x, point.y, point.z);
        GXColor1u32(1);
        GXTexCoord2f32(10.0f * JMACosRadian(angle), 10.0f * MR::sin(angle));
        PSMTXMultVecSR(rotation, &tangent, &tangent);
    }
    GXEnd();
}
