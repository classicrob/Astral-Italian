#pragma once

#include "Game/Screen/ImageEffectBase.hpp"

class JUTTexture;

class DepthOfFieldBlur : public ImageEffectBase {
public:
    DepthOfFieldBlur(const char*);
    virtual void draw() const;

    void createBlurTexture() const;
    void blurTexture(JUTTexture*, s32, s32, f32) const NO_INLINE;
    void drawFinal() const;
    JUTTexture* getFogTexture() const NO_INLINE;

    /* 0x18 */ JUTTexture* mScreenTexture;
    /* 0x1C */ JUTTexture* mOriginalTexture;
    /* 0x20 */ JUTTexture* mReducedTexture;
    /* 0x24 */ JUTTexture* mBlurTexture;
    /* 0x28 */ f32 mIntensity;
    /* 0x2C */ s32 mBlurMaxDist;
    /* 0x30 */ s32 mBlurMinDist;
    /* 0x34 */ JUTTexture* mDepthTexture;
    /* 0x38 */ s32 mFogTextureIndex;
};
