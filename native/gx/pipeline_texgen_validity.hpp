#pragma once
#include "gx.hpp"
#include "shader_info.hpp"
#include <cstdio>

namespace aurora::gx {
// Exactly the source/type combinations accepted by shader.cpp's sampled-texgen
// branch. Emboss generators use embossSrc instead of the ordinary source field.
inline bool petari_texgen_source_valid(const TcgConfig& tcg) {
    if (tcg.type >= GX_TG_BUMP0 && tcg.type <= GX_TG_BUMP7) return true;
    if (tcg.type != GX_TG_MTX2x4 && tcg.type != GX_TG_MTX3x4 && tcg.type != GX_TG_SRTG) return false;
    return (tcg.src >= GX_TG_TEX0 && tcg.src <= GX_TG_TEX7) || tcg.src == GX_TG_POS ||
           tcg.src == GX_TG_NRM || tcg.src == GX_TG_COLOR0 || tcg.src == GX_TG_COLOR1 ||
           tcg.src == GX_TG_BINRM || tcg.src == GX_TG_TANGENT;
}
// Reject malformed array/enum inputs before asking the production analysis which
// texgens are sampled. The row is never repaired or rewritten by this check.
inline bool petari_cached_shader_valid(const ShaderConfig& config, unsigned long long hash) {
    const char* reason = nullptr;
    if (!config.tevStageCount || config.tevStageCount > config.tevStages.size() ||
        config.numIndStages > config.indStages.size()) reason = "stage-count";
    if (unsigned(config.alphaCompare.comp0) > GX_ALWAYS || unsigned(config.alphaCompare.comp1) > GX_ALWAYS ||
        unsigned(config.alphaCompare.op) > GX_AOP_XNOR) reason = "alpha-compare";
    switch (config.fogType) {
    case GX_FOG_NONE: case GX_FOG_PERSP_LIN: case GX_FOG_PERSP_EXP: case GX_FOG_PERSP_EXP2:
    case GX_FOG_PERSP_REVEXP: case GX_FOG_PERSP_REVEXP2: case GX_FOG_ORTHO_LIN: case GX_FOG_ORTHO_EXP:
    case GX_FOG_ORTHO_EXP2: case GX_FOG_ORTHO_REVEXP: case GX_FOG_ORTHO_REVEXP2: break;
    default: reason = "fog-type";
    }
    if (config.attrs[GX_VA_POS].attrType == GX_NONE) reason = "unbound-position";
    for (unsigned i = 0; i < config.attrs.size(); ++i) {
        const auto& attr = config.attrs[i];
        if (attr.attrType == GX_NONE) continue;
        const bool color = i == GX_VA_CLR0 || i == GX_VA_CLR1;
        if (i > GX_VA_TEX7 || attr.attrType > GX_INDEX16 || !attr.cnt || attr.cnt > 9 ||
            attr.compType > (color ? GX_RGBA8 : GX_F32) || (i < GX_VA_POS && attr.attrType != GX_DIRECT))
            reason = "vertex-format";
    }
    for (const auto& swap : config.tevSwapTable)
        if (unsigned(swap.red) > 3 || unsigned(swap.green) > 3 || unsigned(swap.blue) > 3 || unsigned(swap.alpha) > 3)
            reason = "swap-channel";
    if (!reason) for (unsigned i = 0; i < config.numIndStages; ++i) {
        const auto& stage = config.indStages[i];
        if ((stage.texCoordId != GX_TEXCOORD_NULL && unsigned(stage.texCoordId) >= MaxTexCoord) ||
            (stage.texMapId != GX_TEXMAP_NULL && unsigned(stage.texMapId) >= MaxTextures) ||
            unsigned(stage.scaleS) > GX_ITS_256 || unsigned(stage.scaleT) > GX_ITS_256) reason = "indirect-index";
    }
    if (!reason) for (unsigned i = 0; i < config.tevStageCount; ++i) {
        const auto& s = config.tevStages[i];
        const auto color = [](GXTevColorArg a) { return unsigned(a) <= GX_CC_ZERO; };
        const auto alpha = [](GXTevAlphaArg a) { return unsigned(a) <= GX_CA_ZERO; };
        if (!color(s.colorPass.a) || !color(s.colorPass.b) || !color(s.colorPass.c) || !color(s.colorPass.d) ||
            !alpha(s.alphaPass.a) || !alpha(s.alphaPass.b) || !alpha(s.alphaPass.c) || !alpha(s.alphaPass.d) ||
            unsigned(s.colorOp.outReg) >= MaxTevRegs || unsigned(s.alphaOp.outReg) >= MaxTevRegs ||
            unsigned(s.tevSwapTex) >= MaxTevSwap || unsigned(s.tevSwapRas) >= MaxTevSwap ||
            (s.texCoordId != GX_TEXCOORD_NULL && unsigned(s.texCoordId) >= MaxTexCoord) ||
            (s.texMapId != GX_TEXMAP_NULL && unsigned(s.texMapId) >= MaxTextures)) { reason = "stage-index"; break; }
        const auto op = [](const TevOp& value) {
            const auto v = unsigned(value.op);
            return (v <= GX_TEV_SUB || (v >= GX_TEV_COMP_R8_GT && v <= GX_TEV_COMP_RGB8_EQ)) &&
                   unsigned(value.scale) <= GX_CS_DIVIDE_2 && (v >= GX_TEV_COMP_R8_GT || unsigned(value.bias) <= GX_TB_SUBHALF);
        };
        const auto konst = [](unsigned value) { return value <= 7 || (value >= 12 && value <= 31); };
        if (!op(s.colorOp) || !op(s.alphaOp) || !konst(unsigned(s.kcSel)) || !konst(unsigned(s.kaSel)) ||
            (s.channelId != GX_COLOR_NULL && unsigned(s.channelId) > GX_ALPHA_BUMPN) ||
            unsigned(s.indTexAlphaSel) > GX_ITBA_U || unsigned(s.indTexFormat) > GX_ITF_3 ||
            unsigned(s.indTexWrapS) > GX_ITW_0 || unsigned(s.indTexWrapT) > GX_ITW_0) {
            reason = "stage-enum"; break;
        }
        const bool texture = s.colorPass.a == GX_CC_TEXC || s.colorPass.a == GX_CC_TEXA ||
            s.colorPass.b == GX_CC_TEXC || s.colorPass.b == GX_CC_TEXA ||
            s.colorPass.c == GX_CC_TEXC || s.colorPass.c == GX_CC_TEXA ||
            s.colorPass.d == GX_CC_TEXC || s.colorPass.d == GX_CC_TEXA ||
            s.alphaPass.a == GX_CA_TEXA || s.alphaPass.b == GX_CA_TEXA ||
            s.alphaPass.c == GX_CA_TEXA || s.alphaPass.d == GX_CA_TEXA;
        if (texture && s.texMapId != GX_TEXMAP_NULL && s.texCoordId == GX_TEXCOORD_NULL) { reason = "unbound-coordinate"; break; }
    }
    if (!reason) {
        const auto info = build_shader_info(config);
        for (unsigned i = 0; i < config.tcgs.size(); ++i) {
            if (info.sampledTexCoords.test(i) && !petari_texgen_source_valid(config.tcgs[i])) {
                reason = "sampled-texgen-source"; break;
            }
        }
    }
    if (reason) std::fprintf(stderr, "[gx cache reject] hash=%016llx reason=%s\n", hash, reason);
    return reason == nullptr;
}
} // namespace aurora::gx
