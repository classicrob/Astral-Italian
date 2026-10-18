#include "Game/Screen/ImageEffectResource.hpp"
#include "Game/Screen/ImageEffectLocalUtil.hpp"
#include "Game/Util.hpp"
#include "Game/NameObj/NameObjAdaptor.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Screen/BloomEffect.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util/DrawUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SystemUtil.hpp"
#include <JSystem/JUtility/JUTTexture.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>

void MR::connectToSceneImageEffect(NameObj* pObj) {
    MR::connectToScene(pObj, 0x15 /* MovementType_ImageEffect */, MR::CalcAnimType_Environment, MR::DrawBufferType_None, MR::DrawType_ImageEffect);
}

void MR::connectToSceneImageEffectMovement(NameObj* pObj) {
    MR::connectToScene(pObj, 0x15 /* MovementType_ImageEffect */, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_None);
}

void ImageEffectLocalUtil::capture(JUTTexture* pTexture, s32 divisions, s32 tile, GXTexFmt format, bool clear, u8 filter) {
    pTexture->capture((tile % divisions) * MR::getFrameBufferWidth() / divisions, (tile / divisions) * MR::getFrameBufferHeight() / divisions, format,
                      clear, filter);
}

void ImageEffectLocalUtil::setupDrawTexture() {
    Mtx44 projectionMtx;
    C_MTXOrtho(projectionMtx, 0.0f, MR::getFrameBufferHeight(), 0.0f, MR::getFrameBufferWidth(), -10000.0f, 10000.0f);
    GXSetProjection(projectionMtx, GX_ORTHOGRAPHIC);

    MR::loadViewMtxFor2DModel();

    GXClearVtxDesc();
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    TMtx34f posMtx;
    posMtx.identity();
    GXLoadPosMtxImm(posMtx.toMtxPtr(), GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);

    GXSetCullMode(GX_CULL_NONE);
    GXSetClipMode(GX_CLIP_ENABLE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXLoadTexMtxImm(posMtx.toMtxPtr(), GX_TEXMTX0, GX_MTX2x4);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z24X8, 0);
    GXSetNumIndStages(0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTevDirect(GX_TEVSTAGE3);
    GXSetTevDirect(GX_TEVSTAGE4);
    GXSetTevDirect(GX_TEVSTAGE5);
    GXSetTevDirect(GX_TEVSTAGE6);
    GXSetTevDirect(GX_TEVSTAGE7);
    GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, Color8(0, 0, 0, 0));
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetDither(GX_FALSE);
}

void ImageEffectLocalUtil::drawTexture(JUTTexture* pTexture, s32 divisions, s32 tile, u8 intensity, ETexDrawType texDrawType, GXTexMapID textureMap) {
    pTexture->load(textureMap);

    if (texDrawType == TexDrawType_2) {
        GXSetChanMatColor(GX_COLOR0A0, Color8(255, 255, 255, intensity));
        GXSetChanAmbColor(GX_COLOR0A0, Color8(0, 0, 0, 255));
    } else {
        GXSetChanMatColor(GX_COLOR0A0, Color8(intensity, intensity, intensity, 255));
        GXSetChanAmbColor(GX_COLOR0A0, Color8(0, 0, 0, 255));
    }

    switch (texDrawType) {
    case TexDrawType_0:
        GXSetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ZERO, GX_LO_CLEAR);
        break;
    case TexDrawType_1:
        GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
        break;
    case TexDrawType_2:
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
        break;
    case TexDrawType_3:
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCCLR, GX_BL_ZERO, GX_LO_CLEAR);
        break;
    }

    sendTextureVertex(divisions, tile);
}

void ImageEffectLocalUtil::sendTextureVertex(s32 divisions, s32 tile) {
    s32 row = tile / divisions;
    s32 column = tile % divisions;
    f32 width = static_cast< f32 >(MR::getFrameBufferWidth()) / divisions;
    f32 height = static_cast< f32 >(MR::getFrameBufferHeight()) / divisions;
    f32 left = column * width;
    f32 right = (column + 1) * width;
    f32 bottom = (row + 1) * height;
    f32 top = row * height;

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
    GXPosition3f32(left, bottom, 0.0f);
    GXTexCoord2f32(0.0f, 1.0f);

    GXPosition3f32(left, top, 0.0f);
    GXTexCoord2f32(0.0f, 0.0f);

    GXPosition3f32(right, bottom, 0.0f);
    GXTexCoord2f32(1.0f, 1.0f);

    GXPosition3f32(right, top, 0.0f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXEnd();
}

// NONMATCHING: floating-point register/stack allocation and equivalent branch selection differ.
void ImageEffectLocalUtil::blurTexture(JUTTexture* pTexture, s32 divisions, s32 tile, u32 sampleCount, f32 radius, f32 intensity) {
    f32 aspect = MR::isScreen16Per9() ? 1.333f : 1.0f;
    f32 verticalRadius = radius * aspect;
    s32 sampleIntensity = 255.0f * intensity / sampleCount;

    for (u32 i = 0; i < sampleCount; i++) {
        f32 angle = 2.0f * (i * JGeometry::TUtil< f32 >::PI()) / sampleCount;
        TPos3f transMtx;
        transMtx.makeTrans(radius * JMACosRadian(angle), verticalRadius * JMASinRadian(angle), 0.0f);
        GXLoadTexMtxImm(transMtx.toMtxPtr(), GX_TEXMTX0, GX_MTX2x4);
        ETexDrawType type;
        if (i == 0) {
            type = TexDrawType_0;
        } else {
            type = TexDrawType_1;
        }

        drawTexture(pTexture, divisions, tile, sampleIntensity, type);
    }

    TPos3f transMtx;
    transMtx.makeTrans(0.0f, 0.0f, 0.0f);
    GXLoadTexMtxImm(transMtx.toMtxPtr(), GX_TEXMTX0, GX_MTX2x4);
}

void MR::connectToSceneNormalBloom(BloomEffect* pBloomEffect) {
    MR::connectToScene(pBloomEffect, MR::MovementType_None, MR::CalcAnimType_Environment, MR::DrawBufferType_None, MR::DrawType_None);

    NameObjAdaptor* pPreDrawAdaptor = new NameObjAdaptor("BloomEffect::preDraw");

    pPreDrawAdaptor->connectToDraw(MR::Functor(pBloomEffect, &BloomEffect::preDraw));
    MR::connectToScene(pPreDrawAdaptor, MR::MovementType_None, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_BloomEffectPreDraw);

    NameObjAdaptor* pPostDrawAdaptor = new NameObjAdaptor("BloomEffect::postDraw");

    pPostDrawAdaptor->connectToDraw(MR::Functor(pBloomEffect, &BloomEffect::postDraw));
    MR::connectToScene(pPostDrawAdaptor, MR::MovementType_None, MR::CalcAnimType_None, MR::DrawBufferType_None, MR::DrawType_BloomEffectPostDraw);
}

JUTTexture* ImageEffectResource::createTexture(f32 scale, GXTexFmt format, GXTexWrapMode wrap, GXTexFilter filter) {
    JUTTexture* pTexture;

    MR::CurrentHeapRestorer heapRestorer(MR::getSceneHeapGDDR3());

    pTexture = new JUTTexture(scale * MR::getFrameBufferWidth(),
                              scale * MR::getFrameBufferHeight(), format);
    pTexture->mWrapS = wrap;
    pTexture->mWrapT = wrap;
    pTexture->mMinType = filter;
    pTexture->mMagType = filter;
    pTexture->init();

    return pTexture;
}

// NONMATCHING: texture-map and conversion-base registers are swapped.
void ImageEffectLocalUtil::drawFourSampleTexture(JUTTexture* texture, s32 divisions, s32 tile, GXTexMapID textureMap, bool useExtendedStages, f32 radius) {
    Mtx44 projection;
    C_MTXOrtho(projection, 0.0f, MR::getFrameBufferHeight(), 0.0f, MR::getFrameBufferWidth(), -10000.0f, 10000.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    MR::loadViewMtxFor2DModel();
    GXClearVtxDesc();
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetCullMode(GX_CULL_NONE);
    GXSetClipMode(GX_CLIP_ENABLE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX3, GX_FALSE, GX_PTIDENTITY);
    if (useExtendedStages == 0) {
        GXSetNumTevStages(4);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, textureMap, GX_COLOR_NULL);
        GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_2_8);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, textureMap, GX_COLOR_NULL);
        GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_2_8);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD2, textureMap, GX_COLOR_NULL);
        GXSetTevKColorSel(GX_TEVSTAGE2, GX_TEV_KCSEL_2_8);
        GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD3, textureMap, GX_COLOR_NULL);
        GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_2_8);
        GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevKAlphaSel(GX_TEVSTAGE3, GX_TEV_KASEL_8_8);
    } else {
        GXSetNumTevStages(6);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, textureMap, GX_COLOR_NULL);
        GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_8_8);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, textureMap, GX_COLOR_NULL);
        GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_8_8);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD2, textureMap, GX_COLOR_NULL);
        GXSetTevKColorSel(GX_TEVSTAGE2, GX_TEV_KCSEL_8_8);
        GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD3, textureMap, GX_COLOR_NULL);
        GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_8_8);
        GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_ZERO, GX_CC_KONST, GX_CC_TEXC, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE4, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE4, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE4, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE4, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevOrder(GX_TEVSTAGE5, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE5, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE5, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(GX_TEVSTAGE5, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevKAlphaSel(GX_TEVSTAGE5, GX_TEV_KASEL_8_8);
    }
    GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z24X8, 0);
    GXSetNumIndStages(0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTevDirect(GX_TEVSTAGE3);
    GXSetTevDirect(GX_TEVSTAGE4);
    GXSetTevDirect(GX_TEVSTAGE5);
    GXSetTevDirect(GX_TEVSTAGE6);
    GXSetTevDirect(GX_TEVSTAGE7);
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
    f32 aspect = static_cast<f32>(MR::getFrameBufferWidth()) / MR::getFrameBufferHeight();
    matrix.makeTrans(radius, 0.0f, 0.0f);
    GXLoadTexMtxImm(matrix.toMtxPtr(), GX_TEXMTX0, GX_MTX2x4);
    matrix.makeTrans(0.0f, radius * aspect, 0.0f);
    GXLoadTexMtxImm(matrix.toMtxPtr(), GX_TEXMTX1, GX_MTX2x4);
    matrix.makeTrans(-radius, 0.0f, 0.0f);
    GXLoadTexMtxImm(matrix.toMtxPtr(), GX_TEXMTX2, GX_MTX2x4);
    matrix.makeTrans(0.0f, -radius * aspect, 0.0f);
    GXLoadTexMtxImm(matrix.toMtxPtr(), GX_TEXMTX3, GX_MTX2x4);
    texture->load(textureMap);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
    s32 row = tile / divisions;
    s32 column = tile % divisions;
    f32 width = static_cast<f32>(MR::getFrameBufferWidth()) / divisions;
    f32 height = static_cast<f32>(MR::getFrameBufferHeight()) / divisions;
    f32 left = column * width;
    f32 right = (column + 1) * width;
    f32 bottom = (row + 1) * height;
    f32 top = row * height;
    GXPosition3f32(left, bottom, 0.0f);
    GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3f32(left, top, 0.0f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(right, bottom, 0.0f);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(right, top, 0.0f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXEnd();
}

void ImageEffectLocalUtil::setTextureMap(GXTexMapID textureMap) {
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, textureMap, GX_COLOR0A0);
}
