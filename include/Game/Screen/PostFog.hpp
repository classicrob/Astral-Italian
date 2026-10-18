#pragma once
#include "Game/Screen/ImageEffectBase.hpp"
class JUTTexture;
class PostFog : public ImageEffectBase {
public:
    PostFog(const char*);
    virtual ~PostFog();
    virtual void draw() const;
    void captureScreen() const;
    void drawFog(s16 left, s16 top, s16 right, s16 bottom, bool replaceColor) const;
    JUTTexture* getFogTexture() const;
    JUTTexture* mDepthTexture;
    s32 _1C;
    s32 mStartDepth;
    s32 mEndDepth;
    u8 mAlpha;
    u8 mRed;
    u8 mGreen;
    u8 mBlue;
    s32 mFogTextureIndex;
};
