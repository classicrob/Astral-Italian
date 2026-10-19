#include "Game/Screen/BloomEffectSimple.hpp"
#include "Game/Util.hpp"
#include "Game/Screen/CopyFilterNegater.hpp"
#include "Game/Screen/ImageEffectLocalUtil.hpp"
#include "Game/Screen/ImageEffectResource.hpp"
#include "Game/Screen/ImageEffectSystemHolder.hpp"
#include "Game/Util/DrawUtil.hpp"

namespace {
    // static const s32 sLen = _;
    // static const s32 sDivNum = _;
};  // namespace

namespace {
    class TexSpec {
    public:
        TexSpec() {
        }

        void set(JUTTexture*, s32, s32, GXTexFmt);

        /* 0x00 */ JUTTexture* mTexture;
        /* 0x04 */ s32 mDivisions;
        /* 0x08 */ s32 mTile;
        /* 0x0C */ GXTexFmt mFormat;
    };
};  // namespace

BloomEffectSimple::BloomEffectSimple()
    : ImageEffectBase("簡易ブルーム"), mScreenTexture(), mHalfTexture(), mQuarterTextureA(), mQuarterTextureB(), mEighthTextureA(), mEighthTextureB(), mQuarterIntensityTexture(), mMaskFilter(), mThreshold(205), mIntensity(0.3f) {
    MR::connectToSceneImageEffect(this);
    MR::createImageEffectSystemHolder();
    MR::getImageEffectResource()->createBloomTexture();

    mScreenTexture = MR::getImageEffectResource()->mScreenTexture;
    mHalfTexture = MR::getImageEffectResource()->mHalfTexture;
    mQuarterTextureA = MR::getImageEffectResource()->mQuarterTextureA;
    mQuarterTextureB = MR::getImageEffectResource()->mQuarterTextureB;
    mEighthTextureA = MR::getImageEffectResource()->mEighthTextureA;
    mEighthTextureB = MR::getImageEffectResource()->mEighthTextureB;
    mQuarterIntensityTexture = MR::getImageEffectResource()->mQuarterIntensityTexture;
}

void BloomEffectSimple::draw() const {
    if (!isSomething()) {
        return;
    }

    CopyFilterNegater copyFilterNegator;

    ::TexSpec texSpec;
    texSpec.set(mQuarterTextureB, 4, 0, GX_TF_RGB565);

    ImageEffectLocalUtil::capture(texSpec.mTexture, texSpec.mDivisions, texSpec.mTile, texSpec.mFormat, false, 0);
    ImageEffectLocalUtil::capture(mHalfTexture, texSpec.mDivisions, texSpec.mTile, GX_TF_RGB565, true, 0);
    MR::reinitGX();
    ImageEffectLocalUtil::setupDrawTexture();
    ImageEffectLocalUtil::drawTexture(mHalfTexture, 4, 0, 255, ImageEffectLocalUtil::TexDrawType_0);
    ImageEffectLocalUtil::capture(mQuarterTextureA, 4, 0, GX_TF_RGB565, false, 0);

    switch (mMaskFilter) {
    case 0:
        ImageEffectLocalUtil::capture(mQuarterIntensityTexture, 4, 0, GX_TF_I8, false, 0);
        break;
    case 1:
        ImageEffectLocalUtil::capture(mQuarterIntensityTexture, 4, 0, GX_CTF_R8, false, 0);
        break;
    case 2:
        ImageEffectLocalUtil::capture(mQuarterIntensityTexture, 4, 0, GX_CTF_G8, false, 0);
        break;
    case 3:
        ImageEffectLocalUtil::capture(mQuarterIntensityTexture, 4, 0, GX_CTF_B8, false, 0);
        break;
    }

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_RASC, GX_CC_ONE, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_COMP_R8_GT, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
    ImageEffectLocalUtil::drawTexture(mQuarterIntensityTexture, 4, 0, mThreshold, ImageEffectLocalUtil::TexDrawType_3);
    ImageEffectLocalUtil::capture(mEighthTextureA, 4, 0, GX_TF_RGB565, true, 0);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ZERO, GX_BL_ZERO, GX_LO_CLEAR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    ImageEffectLocalUtil::setupDrawTexture();
    texSpec.set(mEighthTextureA, 8, 0, GX_TF_RGB565);
    ImageEffectLocalUtil::blurTexture(texSpec.mTexture, texSpec.mDivisions, texSpec.mTile, 8, 0.009f, 1.0f);
    ImageEffectLocalUtil::capture(mEighthTextureB, texSpec.mDivisions, texSpec.mTile, texSpec.mFormat, false, 0);
    ImageEffectLocalUtil::drawTexture(mQuarterTextureB, 4, 0, 255, ImageEffectLocalUtil::TexDrawType_0);
    const u8 alpha = 255.0f * mIntensity * getFadeRate();
    ImageEffectLocalUtil::drawTexture(mEighthTextureB, 1, 0, alpha, ImageEffectLocalUtil::TexDrawType_1);
}

void ::TexSpec::set(JUTTexture* pParam1, s32 param2, s32 param3, GXTexFmt param4) {
    mTexture = pParam1;
    mDivisions = param2;
    mTile = param3;
    mFormat = param4;
}
