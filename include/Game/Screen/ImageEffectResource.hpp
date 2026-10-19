#pragma once

#include <revolution/gx.h>

class JUTTexture;
class ResourceHolder;

class ImageEffectResource {
public:
    /// @brief Creates a new `ImageEffectResource`.
    ImageEffectResource();

    void createBloomTexture();
    void createBlurTexture();
    void createDOFTexture();
    void createFogTexture();
    void createFogGradationTextures();
    static JUTTexture* createTexture(f32, GXTexFmt, GXTexWrapMode, GXTexFilter);

    /* 0x00 */ JUTTexture* mScreenTexture;
    /* 0x04 */ JUTTexture* mHalfTexture;
    /* 0x08 */ JUTTexture* mHalfIntensityTexture;
    /* 0x0C */ JUTTexture* mQuarterTextureA;
    /* 0x10 */ JUTTexture* mQuarterTextureB;
    /* 0x14 */ JUTTexture* mEighthTextureA;
    /* 0x18 */ JUTTexture* mEighthTextureB;
    /* 0x1C */ JUTTexture* mQuarterIntensityTexture;
    /* 0x20 */ JUTTexture* mQuarterTextureC;
    /* 0x24 */ JUTTexture* mDepthTexture;
    /* 0x28 */ JUTTexture** mFogTextures;
    /* 0x2C */ s32 mFogTextureCount;
    /* 0x30 */ ResourceHolder* mFogResource;
    /* 0x34 */ GXTexFmt mScreenFormat;
};
