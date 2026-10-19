#pragma auto_inline off
#include "Game/Screen/DepthOfFieldBlur.hpp"
#include "Game/Screen/CopyFilterNegater.hpp"
#include "Game/Screen/ImageEffectLocalUtil.hpp"
#include "Game/Screen/ImageEffectResource.hpp"
#include "Game/Screen/ImageEffectSystemHolder.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util/DrawUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include <JSystem/JUtility/JUTTexture.hpp>

DepthOfFieldBlur::DepthOfFieldBlur(const char* name)
    : ImageEffectBase(name), mScreenTexture(nullptr), mOriginalTexture(nullptr), mReducedTexture(nullptr), mBlurTexture(nullptr),
      mIntensity(1.0f), mBlurMaxDist(10000), mBlurMinDist(8200), mDepthTexture(nullptr), mFogTextureIndex(-1) {
    MR::connectToSceneImageEffect(this);
    MR::createImageEffectSystemHolder();
    MR::getImageEffectResource()->createDOFTexture();
    mScreenTexture = MR::getImageEffectResource()->mScreenTexture;
    mOriginalTexture = MR::getImageEffectResource()->mQuarterTextureB;
    mReducedTexture = MR::getImageEffectResource()->mHalfTexture;
    mBlurTexture = MR::getImageEffectResource()->mQuarterTextureA;
    mDepthTexture = MR::getImageEffectResource()->mDepthTexture;
}

void DepthOfFieldBlur::draw() const {
    if (!isSomething()) return;
    CopyFilterNegater negator;
    ImageEffectLocalUtil::capture(mOriginalTexture, 4, 0, GX_TF_RGB565, false, 0);
    ImageEffectLocalUtil::capture(mReducedTexture, 1, 0, GX_TF_RGB565, true, 0);
    ImageEffectLocalUtil::setupDrawTexture();
    createBlurTexture();
    ImageEffectLocalUtil::capture(mDepthTexture, 1, 0, GX_TF_Z16, false, 0);
    drawFinal();
    MR::loadProjectionMtx();
    MR::loadViewMtx();
}

void DepthOfFieldBlur::createBlurTexture() const {
    ImageEffectLocalUtil::drawTexture(mReducedTexture, 4, 0, 255, ImageEffectLocalUtil::TexDrawType_0);
    ImageEffectLocalUtil::capture(mBlurTexture, 4, 0, GX_TF_RGB565, false, 0);
    blurTexture(mBlurTexture, 4, 0, 0.005f * (mIntensity * getFadeRate()));
    ImageEffectLocalUtil::capture(mBlurTexture, 4, 0, GX_TF_RGB565, false, 0);
}

// NONMATCHING: register allocation, stack layout, and arithmetic scheduling differ.
void DepthOfFieldBlur::blurTexture(JUTTexture* texture, s32 divisions, s32 tile, f32 radius) const {
    Mtx44 projection;
    f32 width = MR::getFrameBufferWidth();
    C_MTXOrtho(projection, 0.0f, static_cast<f32>(MR::getFrameBufferHeight()), 0.0f, static_cast<f32>(width), -10000.0f, 10000.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    MR::loadViewMtxFor2DModel();
    GXClearVtxDesc();
    GXSetVtxAttrFmt(static_cast<GXVtxFmt>( 0 ), static_cast<GXAttr>( 9 ), static_cast<GXCompCnt>( 1 ), static_cast<GXCompType>( 4 ), 0);
    GXSetVtxAttrFmt(static_cast<GXVtxFmt>( 0 ), static_cast<GXAttr>( 0xD ), static_cast<GXCompCnt>( 1 ), static_cast<GXCompType>( 4 ), 0);
    GXSetVtxDesc(static_cast<GXAttr>( 9 ), static_cast<GXAttrType>( 1 ));
    GXSetVtxDesc(static_cast<GXAttr>( 0xD ), static_cast<GXAttrType>( 1 ));
    GXSetCullMode(static_cast<GXCullMode>( 0 ));
    GXSetClipMode(static_cast<GXClipMode>( 0 ));
    GXSetNumChans(1);
    GXSetChanCtrl(static_cast<GXChannelID>( 4 ), static_cast<GXBool>( 0 ), static_cast<GXColorSrc>( 0 ), static_cast<GXColorSrc>( 0 ), 0, static_cast<GXDiffuseFn>( 0 ), static_cast<GXAttnFn>( 2 ));
    GXSetNumTexGens(4);
    GXSetTexCoordGen2(static_cast<GXTexCoordID>( 0 ), static_cast<GXTexGenType>( 1 ), static_cast<GXTexGenSrc>( 4 ), 0x1E, static_cast<GXBool>( 0 ), 0x7D);
    GXSetTexCoordGen2(static_cast<GXTexCoordID>( 1 ), static_cast<GXTexGenType>( 1 ), static_cast<GXTexGenSrc>( 4 ), 0x21, static_cast<GXBool>( 0 ), 0x7D);
    GXSetTexCoordGen2(static_cast<GXTexCoordID>( 2 ), static_cast<GXTexGenType>( 1 ), static_cast<GXTexGenSrc>( 4 ), 0x24, static_cast<GXBool>( 0 ), 0x7D);
    GXSetTexCoordGen2(static_cast<GXTexCoordID>( 3 ), static_cast<GXTexGenType>( 1 ), static_cast<GXTexGenSrc>( 4 ), 0x27, static_cast<GXBool>( 0 ), 0x7D);
    GXSetNumTevStages(4);
    GXSetTevOrder(static_cast<GXTevStageID>( 0 ), static_cast<GXTexCoordID>( 0 ), static_cast<GXTexMapID>( 0 ), static_cast<GXChannelID>( 0xFF ));
    GXSetTevKColorSel(static_cast<GXTevStageID>( 0 ), static_cast<GXTevKColorSel>( 6 ));
    GXSetTevColorIn(static_cast<GXTevStageID>( 0 ), static_cast<GXTevColorArg>( 0xF ), static_cast<GXTevColorArg>( 0xE ), static_cast<GXTevColorArg>( 8 ), static_cast<GXTevColorArg>( 0xF ));
    GXSetTevColorOp(static_cast<GXTevStageID>( 0 ), static_cast<GXTevOp>( 0 ), static_cast<GXTevBias>( 0 ), static_cast<GXTevScale>( 0 ), static_cast<GXBool>( 0 ), static_cast<GXTevRegID>( 0 ));
    GXSetTevAlphaIn(static_cast<GXTevStageID>( 0 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 7 ));
    GXSetTevAlphaOp(static_cast<GXTevStageID>( 0 ), static_cast<GXTevOp>( 0 ), static_cast<GXTevBias>( 0 ), static_cast<GXTevScale>( 0 ), static_cast<GXBool>( 0 ), static_cast<GXTevRegID>( 0 ));
    GXSetTevOrder(static_cast<GXTevStageID>( 1 ), static_cast<GXTexCoordID>( 1 ), static_cast<GXTexMapID>( 0 ), static_cast<GXChannelID>( 0xFF ));
    GXSetTevKColorSel(static_cast<GXTevStageID>( 1 ), static_cast<GXTevKColorSel>( 6 ));
    GXSetTevColorIn(static_cast<GXTevStageID>( 1 ), static_cast<GXTevColorArg>( 0xF ), static_cast<GXTevColorArg>( 0xE ), static_cast<GXTevColorArg>( 8 ), static_cast<GXTevColorArg>( 0 ));
    GXSetTevColorOp(static_cast<GXTevStageID>( 1 ), static_cast<GXTevOp>( 0 ), static_cast<GXTevBias>( 0 ), static_cast<GXTevScale>( 0 ), static_cast<GXBool>( 0 ), static_cast<GXTevRegID>( 0 ));
    GXSetTevAlphaIn(static_cast<GXTevStageID>( 1 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 7 ));
    GXSetTevAlphaOp(static_cast<GXTevStageID>( 1 ), static_cast<GXTevOp>( 0 ), static_cast<GXTevBias>( 0 ), static_cast<GXTevScale>( 0 ), static_cast<GXBool>( 0 ), static_cast<GXTevRegID>( 0 ));
    GXSetTevOrder(static_cast<GXTevStageID>( 2 ), static_cast<GXTexCoordID>( 2 ), static_cast<GXTexMapID>( 0 ), static_cast<GXChannelID>( 0xFF ));
    GXSetTevKColorSel(static_cast<GXTevStageID>( 2 ), static_cast<GXTevKColorSel>( 6 ));
    GXSetTevColorIn(static_cast<GXTevStageID>( 2 ), static_cast<GXTevColorArg>( 0xF ), static_cast<GXTevColorArg>( 0xE ), static_cast<GXTevColorArg>( 8 ), static_cast<GXTevColorArg>( 0 ));
    GXSetTevColorOp(static_cast<GXTevStageID>( 2 ), static_cast<GXTevOp>( 0 ), static_cast<GXTevBias>( 0 ), static_cast<GXTevScale>( 0 ), static_cast<GXBool>( 0 ), static_cast<GXTevRegID>( 0 ));
    GXSetTevAlphaIn(static_cast<GXTevStageID>( 2 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 7 ));
    GXSetTevAlphaOp(static_cast<GXTevStageID>( 2 ), static_cast<GXTevOp>( 0 ), static_cast<GXTevBias>( 0 ), static_cast<GXTevScale>( 0 ), static_cast<GXBool>( 0 ), static_cast<GXTevRegID>( 0 ));
    GXSetTevOrder(static_cast<GXTevStageID>( 3 ), static_cast<GXTexCoordID>( 3 ), static_cast<GXTexMapID>( 0 ), static_cast<GXChannelID>( 0xFF ));
    GXSetTevKColorSel(static_cast<GXTevStageID>( 3 ), static_cast<GXTevKColorSel>( 6 ));
    GXSetTevColorIn(static_cast<GXTevStageID>( 3 ), static_cast<GXTevColorArg>( 0xF ), static_cast<GXTevColorArg>( 0xE ), static_cast<GXTevColorArg>( 8 ), static_cast<GXTevColorArg>( 0 ));
    GXSetTevColorOp(static_cast<GXTevStageID>( 3 ), static_cast<GXTevOp>( 0 ), static_cast<GXTevBias>( 0 ), static_cast<GXTevScale>( 0 ), static_cast<GXBool>( 1 ), static_cast<GXTevRegID>( 0 ));
    GXSetTevAlphaIn(static_cast<GXTevStageID>( 3 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 6 ));
    GXSetTevAlphaOp(static_cast<GXTevStageID>( 3 ), static_cast<GXTevOp>( 0 ), static_cast<GXTevBias>( 0 ), static_cast<GXTevScale>( 0 ), static_cast<GXBool>( 0 ), static_cast<GXTevRegID>( 0 ));
    GXSetTevKAlphaSel(static_cast<GXTevStageID>( 3 ), static_cast<GXTevKAlphaSel>( 0 ));
    GXSetZTexture(static_cast<GXZTexOp>( 0 ), static_cast<GXTexFmt>( 0x16 ), 0);
    GXSetNumIndStages(0);
    GXSetTevDirect(static_cast<GXTevStageID>( 0 ));
    GXSetTevDirect(static_cast<GXTevStageID>( 1 ));
    GXSetTevDirect(static_cast<GXTevStageID>( 2 ));
    GXSetTevDirect(static_cast<GXTevStageID>( 3 ));
    GXSetTevDirect(static_cast<GXTevStageID>( 4 ));
    GXSetTevDirect(static_cast<GXTevStageID>( 5 ));
    GXSetTevDirect(static_cast<GXTevStageID>( 6 ));
    GXSetTevDirect(static_cast<GXTevStageID>( 7 ));

    GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, Color8(0, 0, 0, 0));
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetDither(GX_FALSE);
    TPos3f matrix;
    matrix.identity();
    GXLoadPosMtxImm(matrix.toMtxPtr(), GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    f32 height = MR::getFrameBufferHeight();
    f32 aspect = static_cast<f32>(MR::getFrameBufferWidth()) / static_cast<f32>(height);
    matrix.makeTrans(radius, 0.0f, 0.0f);
    GXLoadTexMtxImm(matrix.toMtxPtr(), GX_TEXMTX0, GX_MTX2x4);
    matrix.makeTrans(0.0f, radius * aspect, 0.0f);
    GXLoadTexMtxImm(matrix.toMtxPtr(), GX_TEXMTX1, GX_MTX2x4);
    matrix.makeTrans(-radius, 0.0f, 0.0f);
    GXLoadTexMtxImm(matrix.toMtxPtr(), GX_TEXMTX2, GX_MTX2x4);
    matrix.makeTrans(0.0f, -radius * aspect, 0.0f);
    GXLoadTexMtxImm(matrix.toMtxPtr(), GX_TEXMTX3, GX_MTX2x4);
    texture->load(GX_TEXMAP0);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
    s32 row = tile / divisions;
    s32 column = tile % divisions;
    f32 cellWidth = static_cast<f32>(MR::getFrameBufferWidth()) / divisions;
    f32 cellHeight = static_cast<f32>(MR::getFrameBufferHeight()) / divisions;
    f32 left = column * cellWidth;
    f32 bottom = (row + 1) * cellHeight;
    f32 right = (column + 1) * cellWidth;
    f32 top = row * cellHeight;
    GXPosition3f32(left, bottom, 0.0f); GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3f32(left, top, 0.0f); GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(right, bottom, 0.0f); GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(right, top, 0.0f); GXTexCoord2f32(1.0f, 0.0f);
    GXEnd();
}

// NONMATCHING: register allocation and arithmetic scheduling differ.
void DepthOfFieldBlur::drawFinal() const {
    ImageEffectLocalUtil::drawTexture(mOriginalTexture, 4, 0, 255, ImageEffectLocalUtil::TexDrawType_0);
    GXSetVtxAttrFmt(static_cast<GXVtxFmt>( 0 ), static_cast<GXAttr>( 0xE ), static_cast<GXCompCnt>( 1 ), static_cast<GXCompType>( 4 ), 0);
    GXSetVtxDesc(static_cast<GXAttr>( 0xE ), static_cast<GXAttrType>( 1 ));
    GXSetNumTexGens(3);
    GXSetTexCoordGen2(static_cast<GXTexCoordID>( 0 ), static_cast<GXTexGenType>( 1 ), static_cast<GXTexGenSrc>( 4 ), 0x3C, static_cast<GXBool>( 0 ), 0x7D);
    GXSetTexCoordGen2(static_cast<GXTexCoordID>( 1 ), static_cast<GXTexGenType>( 1 ), static_cast<GXTexGenSrc>( 5 ), 0x3C, static_cast<GXBool>( 0 ), 0x7D);
    GXSetTexCoordGen2(static_cast<GXTexCoordID>( 2 ), static_cast<GXTexGenType>( 1 ), static_cast<GXTexGenSrc>( 4 ), 0x3C, static_cast<GXBool>( 0 ), 0x7D);
    GXSetNumTevStages(2);
    GXSetNumIndStages(1);
    GXSetTevOrder(static_cast<GXTevStageID>( 0 ), static_cast<GXTexCoordID>( 0 ), static_cast<GXTexMapID>( 0 ), static_cast<GXChannelID>( 4 ));
    GXSetTevColorIn(static_cast<GXTevStageID>( 0 ), static_cast<GXTevColorArg>( 0xF ), static_cast<GXTevColorArg>( 0xF ), static_cast<GXTevColorArg>( 0xF ), static_cast<GXTevColorArg>( 8 ));
    GXSetTevColorOp(static_cast<GXTevStageID>( 0 ), static_cast<GXTevOp>( 0 ), static_cast<GXTevBias>( 0 ), static_cast<GXTevScale>( 0 ), static_cast<GXBool>( 1 ), static_cast<GXTevRegID>( 0 ));
    GXSetTevAlphaIn(static_cast<GXTevStageID>( 0 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 6 ));
    GXSetTevAlphaOp(static_cast<GXTevStageID>( 0 ), static_cast<GXTevOp>( 0 ), static_cast<GXTevBias>( 0 ), static_cast<GXTevScale>( 0 ), static_cast<GXBool>( 1 ), static_cast<GXTevRegID>( 0 ));
    GXSetTevKAlphaSel(static_cast<GXTevStageID>( 0 ), static_cast<GXTevKAlphaSel>( 0 ));
    GXSetTevOrder(static_cast<GXTevStageID>( 1 ), static_cast<GXTexCoordID>( 1 ), static_cast<GXTexMapID>( 1 ), static_cast<GXChannelID>( 4 ));
    GXSetTevColorIn(static_cast<GXTevStageID>( 1 ), static_cast<GXTevColorArg>( 0xF ), static_cast<GXTevColorArg>( 0xF ), static_cast<GXTevColorArg>( 0xF ), static_cast<GXTevColorArg>( 0 ));
    GXSetTevColorOp(static_cast<GXTevStageID>( 1 ), static_cast<GXTevOp>( 0 ), static_cast<GXTevBias>( 0 ), static_cast<GXTevScale>( 0 ), static_cast<GXBool>( 1 ), static_cast<GXTevRegID>( 0 ));
    GXSetTevAlphaIn(static_cast<GXTevStageID>( 1 ), static_cast<GXTevAlphaArg>( 7 ), static_cast<GXTevAlphaArg>( 4 ), static_cast<GXTevAlphaArg>( 5 ), static_cast<GXTevAlphaArg>( 7 ));
    GXSetTevAlphaOp(static_cast<GXTevStageID>( 1 ), static_cast<GXTevOp>( 0 ), static_cast<GXTevBias>( 0 ), static_cast<GXTevScale>( 0 ), static_cast<GXBool>( 1 ), static_cast<GXTevRegID>( 0 ));

    GXSetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_0, GX_ITW_OFF, GX_ITW_OFF, GX_FALSE, GX_FALSE, GX_ITBA_OFF);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD2, GX_TEXMAP2);
    s32 maxDist = 10000;
    if (mBlurMaxDist >= 0) maxDist = mBlurMaxDist;
    s32 minDist = 8200;
    if (mBlurMinDist >= 0) minDist = mBlurMinDist;
    f32 nearDepth, farDepth;
    if (maxDist > minDist) { nearDepth = minDist / 10000.0f; farDepth = maxDist / 10000.0f; }
    else { nearDepth = maxDist / 10000.0f; farDepth = minDist / 10000.0f; }
    f32 n = 1.0f - nearDepth;
    f32 f = 1.0f - farDepth;
    nearDepth = 1.0f - n * n;
    farDepth = 1.0f - f * f;
    if (100.0f != MR::getNearZ()) {
        f32 farZ = MR::getFarZ();
        nearDepth = MR::convertDepthForProjection(nearDepth, 100.0f, 800000.0f, MR::getNearZ(), farZ);
        farZ = MR::getFarZ();
        farDepth = MR::convertDepthForProjection(farDepth, 100.0f, 800000.0f, MR::getNearZ(), farZ);
    }
    f32 scale;
    if (0.0f != farDepth - nearDepth) scale = 1.0f / (farDepth - nearDepth);
    else scale = 1.0f;
    f32 offset = -scale * nearDepth;
    s8 exponent = 0;
    while (scale > 0.9990234375f && exponent < 46) { scale *= 0.5f; exponent++; }
    while (0.0f < scale && scale < 0.5f && exponent > -16) { scale *= 2.0f; exponent--; }
    f32 matrix[2][3] = {};
    f32 coefficient = scale;
    coefficient *= 0.00390625f;
    matrix[0][0] = coefficient;
    matrix[0][1] = scale;
    GXSetIndTexMtx(GX_ITM_0, matrix, exponent - 1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    mBlurTexture->load(GX_TEXMAP0);
    getFogTexture()->load(GX_TEXMAP1);
    mDepthTexture->load(GX_TEXMAP2);
    GXSetChanMatColor(GX_COLOR0A0, Color8(255, 255, 255, 255.0f * (mIntensity * getFadeRate())));
    GXSetChanAmbColor(GX_COLOR0A0, Color8(0, 0, 0, 255));
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);
    s32 divisions = 1;
    s32 tile = 0;
    s32 row = tile / divisions;
    s32 column = tile % divisions;
    f32 cellWidth = static_cast<f32>(MR::getFrameBufferWidth()) / divisions;
    f32 cellHeight = static_cast<f32>(MR::getFrameBufferHeight()) / divisions;
    f32 left = column * cellWidth;
    f32 bottom = (row + 1) * cellHeight;
    f32 right = (column + 1) * cellWidth;
    f32 top = row * cellHeight;
    GXPosition3f32(left, bottom, 0.0f); GXTexCoord2f32(0.0f, 1.0f); GXTexCoord2f32(offset, offset);
    GXPosition3f32(left, top, 0.0f); GXTexCoord2f32(0.0f, 0.0f); GXTexCoord2f32(offset, offset);
    GXPosition3f32(right, bottom, 0.0f); GXTexCoord2f32(1.0f, 1.0f); GXTexCoord2f32(offset, offset);
    GXPosition3f32(right, top, 0.0f); GXTexCoord2f32(1.0f, 0.0f); GXTexCoord2f32(offset, offset);
    GXEnd();
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetNumIndStages(0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTevDirect(GX_TEVSTAGE3);
    ImageEffectLocalUtil::setupDrawTexture();
}

JUTTexture* DepthOfFieldBlur::getFogTexture() const {
    s32 index = mFogTextureIndex;
    if (index < 0) return MR::getImageEffectResource()->mFogTextures[0];
    return MR::getImageEffectResource()->mFogTextures[index];
}
