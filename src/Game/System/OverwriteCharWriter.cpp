#include "nw4r/ut/CharWriter.h"
#include "nw4r/ut/Font.h"
#include "Game/Util/SystemUtil.hpp"

namespace {
    inline bool usesPictureFontMapping(const nw4r::ut::Font* font) {
        if (!MR::hasPictureFontNW4R()) {
            return false;
        }
        if (font == MR::getMenuFont96NW4R()) {
            return true;
        }
        return font == MR::getStaffFontNW4R();
    }
}

void nw4r::ut::CharWriter::SetupGX() {
#ifdef __MWERKS__
    if (!((reinterpret_cast< u32 >(this) & 0xFF000000) == 0x80000000 ||
          (reinterpret_cast< u32 >(this) & 0xFF800000) == 0x81000000 ||
          (reinterpret_cast< u32 >(this) & 0xF8000000) == 0x90000000 ||
          (reinterpret_cast< u32 >(this) & 0xFF000000) == 0xC0000000 ||
          (reinterpret_cast< u32 >(this) & 0xFF800000) == 0xC1000000 ||
          (reinterpret_cast< u32 >(this) & 0xF8000000) == 0xD0000000 ||
          (reinterpret_cast< u32 >(this) & 0xFFFFC000) == 0xE0000000)) {
        nw4r::db::Panic("Overwrite.cpp", 1717, "NW4R:Pointer Error\nthis(=%p) is not valid pointer.", this);
    }
    // Original CharWriter.h ResetTextureCache pointer assertion (line80).
    if (!((reinterpret_cast< u32 >(this) & 0xFF000000) == 0x80000000 ||
          (reinterpret_cast< u32 >(this) & 0xFF800000) == 0x81000000 ||
          (reinterpret_cast< u32 >(this) & 0xF8000000) == 0x90000000 ||
          (reinterpret_cast< u32 >(this) & 0xFF000000) == 0xC0000000 ||
          (reinterpret_cast< u32 >(this) & 0xFF800000) == 0xC1000000 ||
          (reinterpret_cast< u32 >(this) & 0xF8000000) == 0xD0000000 ||
          (reinterpret_cast< u32 >(this) & 0xFFFFC000) == 0xE0000000)) {
        nw4r::db::Panic("CharWriter.h", 80, "NW4R:Pointer Error\nthis(=%p) is not valid pointer.", this);
    }
#endif
    // A null font is valid: the original method selects SetupGXDefault below.
    ResetTextureCache();

    if (usesPictureFontMapping(mFont)) {
        static const nw4r::ut::Color fog = 0;
        GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, fog);
        GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
        GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z8, 0);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetChanCtrl(GX_COLOR1A1, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetNumTexGens(1);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, 0x7D);
        GXSetNumIndStages(0);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
        GXSetNumTevStages(2);
        GXSetTevDirect(GX_TEVSTAGE0);
        GXSetTevDirect(GX_TEVSTAGE1);
        GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);

        GXColorS10 bias = {0, 0, 0, 0};
        bias.r = -static_cast< s32 >(mColorMapping.max.r) / 2;
        bias.g = -static_cast< s32 >(mColorMapping.max.g) / 2;
        bias.b = -static_cast< s32 >(mColorMapping.max.b) / 2;
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetTevColor(GX_TEVREG0, mColorMapping.min);
        GXSetTevColor(GX_TEVREG1, mColorMapping.max);
        GXSetTevColorS10(GX_TEVREG2, bias);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_C1, GX_CC_TEXC, GX_CC_C2);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE, GX_TEVPREV);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_RASC, GX_CC_ZERO);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        SetupVertexFormat();
        return;
    }

    if ((mColorMapping.min != 0x00000000UL) || (mColorMapping.max != 0xFFFFFFFFUL)) {
        SetupGXWithColorMapping(mColorMapping.min, mColorMapping.max);
    } else if (mFont) {
        GXTexFmt format = mFont->GetTextureFormat();
        switch (format) {
        case GX_TF_I4:
        case GX_TF_I8:
            SetupGXForI();
            break;
        case GX_TF_IA4:
        case GX_TF_IA8:
            SetupGXDefault();
            break;
        case GX_TF_RGB565:
        case GX_TF_RGB5A3:
        case GX_TF_RGBA8:
            SetupGXForRGBA();
            break;
        default:
            nw4r::db::Warning("Overwrite.cpp", 1759, "CharWriter::SetupGX: Unknown font sheet format(=%d)", format);
            SetupGXDefault();
            break;
        }
    } else {
        SetupGXDefault();
    }
}
