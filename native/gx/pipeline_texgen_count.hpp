#pragma once
#include "gx.hpp"

namespace aurora::gx {
// Differential oracle: Dolphin 409881c34357d0f105fd473167d15ab0bd9c1628,
// PixelShaderGen.cpp:1047-1052,1274-1291. Normalize the copied pipeline,
// never the live GX registers or inactive texgen sentinels.
inline void petari_tev_texgen_count(ShaderConfig& config, unsigned numTexGens) {
  for (unsigned i = 0; i < config.tevStageCount; ++i) {
    auto& stage = config.tevStages[i];
    if (numTexGens == 0) {
      // Dolphin supplies black/zero alpha even when the TEV order is disabled;
      // it does not perform the direct texture sample at coordinate (0,0).
      const auto color = [](GXTevColorArg arg) {
        return arg == GX_CC_TEXC || arg == GX_CC_TEXA ? GX_CC_ZERO : arg;
      };
      stage.colorPass.a = color(stage.colorPass.a);
      stage.colorPass.b = color(stage.colorPass.b);
      stage.colorPass.c = color(stage.colorPass.c);
      stage.colorPass.d = color(stage.colorPass.d);
      const auto alpha = [](GXTevAlphaArg arg) { return arg == GX_CA_TEXA ? GX_CA_ZERO : arg; };
      stage.alphaPass.a = alpha(stage.alphaPass.a);
      stage.alphaPass.b = alpha(stage.alphaPass.b);
      stage.alphaPass.c = alpha(stage.alphaPass.c);
      stage.alphaPass.d = alpha(stage.alphaPass.d);
      stage.texMapId = GX_TEXMAP_NULL;
      stage.texCoordId = GX_TEXCOORD_NULL;
    } else if (stage.texCoordId != GX_TEXCOORD_NULL && unsigned(stage.texCoordId) >= numTexGens) {
      stage.texCoordId = GX_TEXCOORD0;
    }
  }
}
} // namespace aurora::gx
