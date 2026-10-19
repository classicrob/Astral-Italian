#pragma once

#include "Game/Screen/ImageEffectBase.hpp"

class JUTTexture;

class BloomEffectSimple : public ImageEffectBase {
public:
    /// @brief Creates a new `BloomEffectSimple`.
    BloomEffectSimple();

    /// @brief Draws the `BloomEffectSimple` to the screen.
    virtual void draw() const;

    /* 0x14 */ JUTTexture* mScreenTexture;
    /* 0x18 */ JUTTexture* mHalfTexture;
    /* 0x1C */ JUTTexture* mQuarterTextureA;
    /* 0x20 */ JUTTexture* mQuarterTextureB;
    /* 0x24 */ JUTTexture* mEighthTextureA;
    /* 0x28 */ JUTTexture* mEighthTextureB;
    /* 0x2C */ JUTTexture* mQuarterIntensityTexture;
    /* 0x30 */ u32 mMaskFilter;
    /* 0x34 */ u8 mThreshold;
    /* 0x38 */ f32 mIntensity;
};
