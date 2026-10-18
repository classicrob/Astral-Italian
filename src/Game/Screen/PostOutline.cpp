#include "Game/Screen/PostOutline.hpp"
#include "Game/Screen/CopyFilterNegater.hpp"
#include "Game/Screen/ImageEffectLocalUtil.hpp"
#include "Game/Screen/ImageEffectResource.hpp"
#include "Game/Screen/ImageEffectSystemHolder.hpp"
#include "Game/Util.hpp"
#include "Game/Util/Color.hpp"
#include <JSystem/JUtility/JUTTexture.hpp>

PostOutline::PostOutline(const char* name) : ImageEffectBase(name), mScreenTexture(nullptr), mOriginalTexture(nullptr), mBlurTexture(nullptr) {
    MR::connectToSceneImageEffect(this);
    MR::createImageEffectSystemHolder();
    mScreenTexture = ImageEffectResource::createTexture(0.5f, GX_TF_RGBA8, GX_CLAMP, GX_LINEAR);
    mOriginalTexture = ImageEffectResource::createTexture(1.0f, GX_TF_I4, GX_CLAMP, GX_LINEAR);
    mBlurTexture = ImageEffectResource::createTexture(0.5f, GX_TF_I8, GX_CLAMP, GX_LINEAR);
}

void PostOutline::draw() const {
    if (!isSomething()) return;
    captureScreen();
    ImageEffectLocalUtil::drawFourSampleTexture(mOriginalTexture, 2, 0, GX_TEXMAP0, false, 0.005f);
    ImageEffectLocalUtil::capture(mBlurTexture, 2, 0, GX_TF_I8, false, 0);
    ImageEffectLocalUtil::setupDrawTexture();
    ImageEffectLocalUtil::drawTexture(mScreenTexture, 2, 0, 255, ImageEffectLocalUtil::TexDrawType_0);
    drawOutline(0, 0, MR::getFrameBufferWidth(), MR::getFrameBufferHeight(), true);
}

void PostOutline::captureScreen() const {
    CopyFilterNegater negater;
    GXSetCopyClamp(static_cast<GXFBClamp>(GX_CLAMP_TOP | GX_CLAMP_BOTTOM));
    ImageEffectLocalUtil::capture(mScreenTexture, 1, 0, GX_TF_RGBA8, false, 0);
    ImageEffectLocalUtil::capture(mOriginalTexture, 1, 0, GX_TF_I4, false, 0);
}

void PostOutline::drawOutline(s16 left, s16 top, s16 right, s16 bottom, bool useAlpha) const {
    ImageEffectLocalUtil::setupDrawTexture();
    Mtx44 projection;
    C_MTXOrtho(projection, 0.0f, MR::getFrameBufferHeight(), 0.0f, MR::getFrameBufferWidth(), -10000.0f, 10000.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    MR::loadViewMtxFor2DModel();
    TMtx34f matrix;
    GXClearVtxDesc();
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    matrix.identity();
    GXLoadPosMtxImm(matrix.toMtxPtr(), GX_PNMTX0);
    GXSetCurrentMtx(0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetClipMode(GX_CLIP_ENABLE);
    GXSetNumChans(0);
    GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, Color8(0, 0, 0, 0));
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetDither(GX_TRUE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetNumTexGens(2);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
    GXSetTexCoordBias(GX_TEXCOORD0, GX_FALSE, GX_FALSE);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordScaleManually(GX_TEXCOORD1, GX_FALSE, 0, 0);
    GXSetTexCoordBias(GX_TEXCOORD1, GX_FALSE, GX_FALSE);
    GXSetNumTevStages(3);
    GXSetNumIndStages(0);
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
    if (useAlpha != 0) {
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
        GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_8_8);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_APREV);
        GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_8_8);
        GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
        GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_COMP_RGB8_GT, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_APREV, GX_CA_A0, GX_CA_A1, GX_CA_ZERO);
    } else {
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_8_8);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_8_8);
        GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
        GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_COMP_RGB8_GT, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_CPREV, GX_CC_C0, GX_CC_ONE, GX_CC_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevKAlphaSel(GX_TEVSTAGE2, GX_TEV_KASEL_8_8);
    }
    GXSetTevColor(GX_TEVREG0, Color8(mThreshold, mThreshold, mThreshold, mThreshold));
    u8 intensity = static_cast<s32>(255.0f * get_10());
    GXSetTevColor(GX_TEVREG1, Color8(intensity, intensity, intensity, intensity));
    mBlurTexture->load(GX_TEXMAP0);
    mOriginalTexture->load(GX_TEXMAP1);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);
    GXPosition3s16(left, bottom, 0);
    GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3s16(left, top, 0);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3s16(right, bottom, 0);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3s16(right, top, 0);
    GXTexCoord2f32(1.0f, 0.0f);
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

PostOutline::~PostOutline() {}
