#include "Game/Screen/PostFog.hpp"
#include "Game/Screen/CopyFilterNegater.hpp"
#include "Game/Screen/ImageEffectLocalUtil.hpp"
#include "Game/Screen/ImageEffectResource.hpp"
#include "Game/Screen/ImageEffectSystemHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/Color.hpp"
#include <JSystem/JUtility/JUTTexture.hpp>

PostFog::PostFog(const char* name) : ImageEffectBase(name), mDepthTexture(nullptr), _1C(0), mStartDepth(0), mEndDepth(10000), mAlpha(255), mFogTextureIndex(-1) {
    MR::connectToSceneImageEffect(this);
    MR::createImageEffectSystemHolder();
    MR::getImageEffectResource()->createFogTexture();
    mDepthTexture = MR::getImageEffectResource()->mDepthTexture;
    mRed = 255;
    mGreen = 255;
    mBlue = 255;
}

void PostFog::draw() const {
    if (!isSomething()) return;
    captureScreen();
    ImageEffectLocalUtil::setupDrawTexture();
    ImageEffectLocalUtil::drawTexture(MR::getImageEffectResource()->mHalfTexture, 2, 0, 255, ImageEffectLocalUtil::TexDrawType_0);
    drawFog(0, 0, MR::getFrameBufferWidth(), MR::getFrameBufferHeight(), false);
}

void PostFog::captureScreen() const {
    CopyFilterNegater negater;
    GXSetCopyClamp(static_cast<GXFBClamp>(GX_CLAMP_TOP | GX_CLAMP_BOTTOM));
    ImageEffectLocalUtil::capture(MR::getImageEffectResource()->mHalfTexture, 1, 0, GX_TF_RGB565, false, 0);
    ImageEffectLocalUtil::capture(mDepthTexture, 1, 0, GX_TF_Z16, false, 0);
}

// NONMATCHING: one floating-point multiplication has commuted operands.
void PostFog::drawFog(s16 left, s16 top, s16 right, s16 bottom, bool replaceColor) const {
    Mtx44 projection;
    C_MTXOrtho(projection, 0.0f, MR::getFrameBufferHeight(), 0.0f, MR::getFrameBufferWidth(), -10000.0f, 10000.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    MR::loadViewMtxFor2DModel();
    TMtx34f matrix;
    GXClearVtxDesc();
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX1, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX1, GX_DIRECT);
    matrix.identity();
    GXLoadPosMtxImm(matrix.toMtxPtr(), GX_PNMTX0);
    GXSetCurrentMtx(0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetClipMode(GX_CLIP_ENABLE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetChanMatColor(GX_COLOR0A0, Color8(mRed, mGreen, mBlue, 255));
    GXSetChanAmbColor(GX_COLOR0A0, Color8(0, 0, 0, 255));
    GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, Color8(0, 0, 0, 0));
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetDither(GX_TRUE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    f32 nearDepth, farDepth;
    if (mEndDepth > mStartDepth) {
        nearDepth = mStartDepth / 10000.0f;
        farDepth = mEndDepth / 10000.0f;
    } else {
        nearDepth = mEndDepth / 10000.0f;
        farDepth = mStartDepth / 10000.0f;
    }
    nearDepth = 1.0f - nearDepth;
    farDepth = 1.0f - farDepth;
    nearDepth *= nearDepth;
    farDepth *= farDepth;
    f32 start = 1.0f - nearDepth;
    f32 end = 1.0f - farDepth;
    if (100.0f != MR::getNearZ()) {
        start = MR::convertDepthForProjection(start, 100.0f, 800000.0f, MR::getNearZ(), MR::getFarZ());
        end = MR::convertDepthForProjection(end, 100.0f, 800000.0f, MR::getNearZ(), MR::getFarZ());
        if (end - start < 0.001953f) start = end - 0.001953f;
    }
    f32 scale;
    if (0.0f != end - start) scale = 1.0f / (end - start);
    else scale = 1.0f;
    f32 offset = -scale * start;
    GXSetNumTexGens(2);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
    GXSetTexCoordBias(GX_TEXCOORD0, GX_FALSE, GX_FALSE);
    GXSetNumTevStages(1);
    GXSetNumIndStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
    GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevIndirect(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_NONE, GX_ITM_0, GX_ITW_OFF, GX_ITW_0, GX_FALSE, GX_FALSE, GX_ITBA_OFF);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP0);
    s8 exponent = 0;
    while (scale > 0.9990234375f && exponent < 46) {
        scale *= 0.5f;
        exponent++;
    }
    while (0.0f < scale && scale < 0.5f && exponent > -17) {
        scale *= 2.0f;
        exponent--;
    }
    f32 indirectMatrix[2][3] = {};
    indirectMatrix[0][1] = scale;
    indirectMatrix[0][0] = scale / 256.0f;
    GXSetIndTexMtx(GX_ITM_0, indirectMatrix, exponent - 1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    u8 intensity = static_cast<s32>(mAlpha * getFadeRate());
    GXSetTevKColor(GX_KCOLOR0, Color8(intensity, intensity, intensity, intensity));
    if (replaceColor) {
        GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
    }
    mDepthTexture->load(GX_TEXMAP0);
    getFogTexture()->load(GX_TEXMAP1);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);
    GXPosition3s16(left, bottom, 0); GXTexCoord2f32(0.0f, 1.0f); GXTexCoord2f32(offset, offset);
    GXPosition3s16(left, top, 0); GXTexCoord2f32(0.0f, 0.0f); GXTexCoord2f32(offset, offset);
    GXPosition3s16(right, bottom, 0); GXTexCoord2f32(1.0f, 1.0f); GXTexCoord2f32(offset, offset);
    GXPosition3s16(right, top, 0); GXTexCoord2f32(1.0f, 0.0f); GXTexCoord2f32(offset, offset);
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

JUTTexture* PostFog::getFogTexture() const {
    s32 index = mFogTextureIndex;
    if (index < 0) return MR::getImageEffectResource()->mFogTextures[0];
    return MR::getImageEffectResource()->mFogTextures[index];
}

PostFog::~PostFog() {}
