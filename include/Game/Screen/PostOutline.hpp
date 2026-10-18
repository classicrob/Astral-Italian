#pragma once
#include "Game/Screen/ImageEffectBase.hpp"
class JUTTexture;
class PostOutline : public ImageEffectBase {
public:
    PostOutline(const char*);
    virtual ~PostOutline();
    virtual void draw() const;
    void captureScreen() const;
    void drawOutline(s16 left, s16 top, s16 right, s16 bottom, bool useAlpha) const;
    JUTTexture* mScreenTexture;
    JUTTexture* mOriginalTexture;
    JUTTexture* mBlurTexture;
    u8 mThreshold;
};
