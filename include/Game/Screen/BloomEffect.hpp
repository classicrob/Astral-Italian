#pragma once

#include "Game/Screen/ImageEffectBase.hpp"

class JUTTexture;

class BloomEffect : public ImageEffectBase {
public:
    enum BLOOM_TEX_DRAW_TYPE {
        DrawType_0 = 0,
        DrawType_1 = 1,
        DrawType_2 = 2,
    };

    BloomEffect(const char* pName);

    virtual ~BloomEffect() {
    }

    void preDraw() const;
    void postDraw() const;
    u8 getIntensity1Default() const;
    u8 getIntensity2Default() const;
    bool isForEachModel() const;
    void initDraw() const;
    void drawTexture(JUTTexture*, s32, s32, u8, BLOOM_TEX_DRAW_TYPE) const;
    void blurTexture(JUTTexture*, Mtx*, s32, s32, u32, f32, u32, const f32*) const;
    void initBlurMtx(Mtx*, u32, f32, f32) const;
    void initBlur(JUTTexture*, u32, f32) const;
    void drawBlur(s32, s32, Mtx*, u32, u32) const;

    /* 0x14 */ u8 mBloomIntensity;
    /* 0x18 */ f32 mBlurIntensity1;
    /* 0x1C */ f32 mBlurIntensity2;
    /* 0x20 */ u8 mThreshold;
    /* 0x24 */ JUTTexture* mScreenTexture;
    /* 0x28 */ JUTTexture* mHalfTexture;
    /* 0x2C */ JUTTexture* mQuarterTextureA;
    /* 0x30 */ JUTTexture* mQuarterTextureB;
    /* 0x34 */ JUTTexture* mEighthTextureA;
    /* 0x38 */ JUTTexture* mEighthTextureB;
    /* 0x3C */ JUTTexture* mCompositeTexture;
    /* 0x40 */ JUTTexture* mQuarterIntensityTexture;
    /* 0x44 */ JUTTexture* mQuarterTextureC;
    /* 0x48 */ Mtx* mBlurMtx1;
    /* 0x4C */ Mtx* mBlurMtx2;
};
