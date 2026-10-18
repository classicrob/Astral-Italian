#pragma once

#include <revolution/gx/GXEnum.h>

class BloomEffect;
class JUTTexture;
class NameObj;

namespace ImageEffectLocalUtil {
    enum ETexDrawType {
        TexDrawType_0,
        TexDrawType_1,
        TexDrawType_2,
        TexDrawType_3,
    };

    void drawFourSampleTexture(JUTTexture*, s32 divisions, s32 tile, GXTexMapID, bool useExtendedStages, f32 radius);
    void capture(JUTTexture* pTexture, s32 divisions, s32 tile, GXTexFmt format, bool clear, u8 filter);
    void setupDrawTexture();
    void setTextureMap(GXTexMapID textureMap);
    void drawTexture(JUTTexture* pTexture, s32 divisions, s32 tile, u8 intensity, ETexDrawType texDrawType, GXTexMapID textureMap = GX_TEXMAP0);
    void sendTextureVertex(s32 divisions, s32 tile);
    void blurTexture(JUTTexture* pTexture, s32 divisions, s32 tile, u32 sampleCount, f32 radius, f32 intensity);
};  // namespace ImageEffectLocalUtil

namespace MR {
    void connectToSceneImageEffect(NameObj* pObj);
    void connectToSceneImageEffectMovement(NameObj* pObj);
    void connectToSceneNormalBloom(BloomEffect* pBloomEffect);
};  // namespace MR
