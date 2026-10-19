#include "Game/Screen/ImageEffectResource.hpp"
#include "Game/Util.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <JSystem/JUtility/JUTTexture.hpp>
#include <cstdio>

namespace {
    // Stage that renders the screen texture as RGB565 instead of RGBA8.
    const char* sRGB565ScreenStageName = "AndoYoshiStage";
}

// NONMATCHING: zero initializer register differs (r4 versus r5).
ImageEffectResource::ImageEffectResource()
    : mScreenTexture(nullptr), mHalfTexture(nullptr), mHalfIntensityTexture(nullptr), mQuarterTextureA(nullptr), mQuarterTextureB(nullptr), mEighthTextureA(nullptr), mEighthTextureB(nullptr), mQuarterIntensityTexture(nullptr), mQuarterTextureC(nullptr), mDepthTexture(nullptr), mFogTextures(nullptr), mFogTextureCount(0), mFogResource(nullptr), mScreenFormat(GX_TF_RGBA8) {
    if (MR::isEqualStageName(sRGB565ScreenStageName)) mScreenFormat = GX_TF_RGB565;
}

void ImageEffectResource::createBloomTexture() {
    if (mScreenTexture == nullptr) {
        mScreenTexture = createTexture(1.0f, mScreenFormat, GX_CLAMP, GX_LINEAR);
    }

    if (mHalfTexture == nullptr) {
        mHalfTexture = createTexture(0.5f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (mQuarterTextureA == nullptr) {
        mQuarterTextureA = createTexture(0.25f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (mQuarterTextureB == nullptr) {
        mQuarterTextureB = createTexture(0.25f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (mEighthTextureA == nullptr) {
        mEighthTextureA = createTexture(0.125f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (mEighthTextureB == nullptr) {
        mEighthTextureB = createTexture(0.125f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (mQuarterIntensityTexture == nullptr) {
        mQuarterIntensityTexture = createTexture(0.25f, GX_TF_I8, GX_CLAMP, GX_LINEAR);
    }

    if (mQuarterTextureC == nullptr) {
        mQuarterTextureC = createTexture(0.25f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }
}

void ImageEffectResource::createBlurTexture() {
    if (mScreenTexture == nullptr) {
        mScreenTexture = createTexture(1.0f, mScreenFormat, GX_CLAMP, GX_LINEAR);
    }

    if (mHalfIntensityTexture == nullptr) {
        mHalfIntensityTexture = createTexture(0.5f, GX_TF_I8, GX_CLAMP, GX_LINEAR);
    }
}

void ImageEffectResource::createDOFTexture() {
    if (mScreenTexture == nullptr) {
        mScreenTexture = createTexture(1.0f, mScreenFormat, GX_CLAMP, GX_LINEAR);
    }

    if (mHalfTexture == nullptr) {
        mHalfTexture = createTexture(0.5f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (mQuarterTextureA == nullptr) {
        mQuarterTextureA = createTexture(0.25f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (mQuarterTextureB == nullptr) {
        mQuarterTextureB = createTexture(0.25f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (mDepthTexture == nullptr) {
        mDepthTexture = createTexture(1.0f, GX_TF_IA8, GX_CLAMP, GX_NEAR);
    }
    createFogGradationTextures();
}


void ImageEffectResource::createFogTexture() {
    if (mHalfTexture == nullptr) mHalfTexture = createTexture(0.5f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    if (mDepthTexture == nullptr) mDepthTexture = createTexture(1.0f, GX_TF_IA8, GX_CLAMP, GX_NEAR);
    createFogGradationTextures();
}

void ImageEffectResource::createFogGradationTextures() {
    if (mFogResource) return;
    mFogResource = MR::createAndAddResourceHolder("PostFog.arc");
    mFogTextureCount = 0;
    while (true) {
        if (!MR::isExistFileInArc(mFogResource, "GradType%d.bti", mFogTextureCount)) break;
        mFogTextureCount++;
    }
    mFogTextures = new JUTTexture*[mFogTextureCount];
    for (s32 i = 0; i < mFogTextureCount; i++) {
        char name[256];
        snprintf(name, sizeof(name), "GradType%d.bti", i);
        mFogTextures[i] = new JUTTexture(MR::getTextureResource(mFogResource, name), 0);
        mFogTextures[i]->mWrapS = GX_CLAMP;
        mFogTextures[i]->mWrapT = GX_CLAMP;
        mFogTextures[i]->mMinType = GX_LINEAR;
        mFogTextures[i]->mMagType = GX_LINEAR;
        mFogTextures[i]->init();
    }
}
