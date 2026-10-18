#include "Game/Screen/ImageEffectResource.hpp"
#include "Game/Util.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <JSystem/JUtility/JUTTexture.hpp>
#include <cstdio>

extern "C" const char* lbl_807D0218;

// NONMATCHING: zero initializer register differs (r4 versus r5).
ImageEffectResource::ImageEffectResource()
    : _0(nullptr), _4(nullptr), _8(nullptr), _C(nullptr), _10(nullptr), _14(nullptr), _18(nullptr), _1C(nullptr), _20(nullptr), _24(nullptr), mFogTextures(nullptr), mFogTextureCount(0), mFogResource(nullptr), mScreenFormat(GX_TF_RGBA8) {
    if (MR::isEqualStageName(lbl_807D0218)) mScreenFormat = GX_TF_RGB565;
}

void ImageEffectResource::createBloomTexture() {
    if (_0 == nullptr) {
        _0 = createTexture(1.0f, mScreenFormat, GX_CLAMP, GX_LINEAR);
    }

    if (_4 == nullptr) {
        _4 = createTexture(0.5f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (_C == nullptr) {
        _C = createTexture(0.25f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (_10 == nullptr) {
        _10 = createTexture(0.25f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (_14 == nullptr) {
        _14 = createTexture(0.125f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (_18 == nullptr) {
        _18 = createTexture(0.125f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (_1C == nullptr) {
        _1C = createTexture(0.25f, GX_TF_I8, GX_CLAMP, GX_LINEAR);
    }

    if (_20 == nullptr) {
        _20 = createTexture(0.25f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }
}

void ImageEffectResource::createBlurTexture() {
    if (_0 == nullptr) {
        _0 = createTexture(1.0f, mScreenFormat, GX_CLAMP, GX_LINEAR);
    }

    if (_8 == nullptr) {
        _8 = createTexture(0.5f, GX_TF_I8, GX_CLAMP, GX_LINEAR);
    }
}

void ImageEffectResource::createDOFTexture() {
    if (_0 == nullptr) {
        _0 = createTexture(1.0f, mScreenFormat, GX_CLAMP, GX_LINEAR);
    }

    if (_4 == nullptr) {
        _4 = createTexture(0.5f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (_C == nullptr) {
        _C = createTexture(0.25f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (_10 == nullptr) {
        _10 = createTexture(0.25f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    }

    if (_24 == nullptr) {
        _24 = createTexture(1.0f, GX_TF_IA8, GX_CLAMP, GX_NEAR);
    }
    createFogGradationTextures();
}


void ImageEffectResource::createFogTexture() {
    if (_4 == nullptr) _4 = createTexture(0.5f, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    if (_24 == nullptr) _24 = createTexture(1.0f, GX_TF_IA8, GX_CLAMP, GX_NEAR);
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
