#include "Game/Screen/ImageEffectResource.hpp"
#include "Game/Map/PauseBlur.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/CopyFilterNegater.hpp"
#include "Game/Screen/ImageEffectLocalUtil.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ObjUtil.hpp"
#include <revolution/gx.h>

class JUTTexture;

// Unnamed SMG2 helpers / ImageEffectResource::createTexture (static in the target: f1 = scale, r3 = GXTexFmt, r4/r5 = flags).

extern "C" void fn_80476C60(s32);
extern "C" void fn_80477230(JUTTexture*, s32, s32, s32, f32, s32);

namespace MR {
    void incrementPauseBlurDelay();
    void decrementPauseBlurDelay();
    void requestPauseBlurMovementOn();
}  // namespace MR

void decrementPauseBlurDelayCount(PauseBlur* pBlur) {
    if (pBlur->mDelay > 0) {
        pBlur->mDelay--;
    }
}

// Blur sequence, drawn in several passes depending on the timer.
void drawBlurTexture(const PauseBlur* pBlur) {
    ImageEffectLocalUtil::setupDrawTexture();
    ImageEffectLocalUtil::capture(pBlur->mTexture1, pBlur->mResolution, 0, GX_TF_RGB565, false, 0);

    if (pBlur->mTimer > 1) {
        ImageEffectLocalUtil::drawTexture(pBlur->mTexture2, pBlur->mResolution, 0, 255, ImageEffectLocalUtil::TexDrawType_0);
    }

    ImageEffectLocalUtil::capture(pBlur->mTexture2, pBlur->mHalfResolution, pBlur->mTimer - 1, GX_TF_RGB565, true, 0);

    s32 n = pBlur->mTimer - 1;
    s32 offsetRow = n / pBlur->mHalfResolution;
    s32 offset = (n - offsetRow * pBlur->mHalfResolution) + pBlur->mHalfResolution * pBlur->mResolution * offsetRow;

    fn_80476C60(1);
    ImageEffectLocalUtil::drawTexture(pBlur->mTexture2, pBlur->mHalfResolution * pBlur->mResolution, offset, 255,
                                      ImageEffectLocalUtil::TexDrawType_0);

    ImageEffectLocalUtil::capture(pBlur->mTexture2, pBlur->mResolution, 0, GX_TF_RGB565, false, 0);

    if (pBlur->mTimer == pBlur->mSquareResolution) {
        fn_80477230(pBlur->mTexture2, pBlur->mResolution, 0, 2, 0.005f, 0);
        ImageEffectLocalUtil::capture(pBlur->mTexture2, pBlur->mResolution, 0, GX_TF_RGB565, false, 0);
        fn_80477230(pBlur->mTexture2, pBlur->mResolution, 0, 3, 0.01f, 0);
        ImageEffectLocalUtil::capture(pBlur->mTexture2, pBlur->mResolution, 0, GX_TF_RGB565, false, 0);
    }

    ImageEffectLocalUtil::setupDrawTexture();
    ImageEffectLocalUtil::drawTexture(pBlur->mTexture1, pBlur->mResolution, 0, 255, ImageEffectLocalUtil::TexDrawType_0);
}

PauseBlur::PauseBlur(const char* pName) : NameObj(pName) {
    mAlpha = 0;
    mTimer = 0;
    mResolution = 4;
    mHalfResolution = mResolution >> 1;
    mSquareResolution = mHalfResolution * mHalfResolution;
    mDelay = 0;
    mIsActive = false;

    mTexture1 = ImageEffectResource::createTexture(1.0f / mResolution, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);
    mTexture2 = ImageEffectResource::createTexture(1.0f / mResolution, GX_TF_RGB565, GX_CLAMP, GX_LINEAR);

    MR::connectToScene(this, 0x1A, -1, -1, 0x42);
}

void PauseBlur::init(const JMapInfoIter&) {
}

void PauseBlur::movement() {
    if (mDelay == 0) {
        if (!mIsActive) {
            return;
        }

        if (mAlpha != 0 && mAlpha > 0x23) {
            mAlpha -= 0x23;
            return;
        }

        mAlpha = 0;
        mTimer = 0;
        mIsActive = false;
        return;
    }

    if (!mIsActive) {
        mIsActive = true;
    }

    if (mTimer < mSquareResolution + 1) {
        mTimer++;
        return;
    }

    if (mAlpha >= 0xFF) {
        return;
    }

    if (0xFF - mAlpha < 0x23) {
        mAlpha = 0xFF;
    } else {
        mAlpha += 0x23;
    }
}

void PauseBlur::draw() const {
    if (!mIsActive) {
        return;
    }

    CopyFilterNegater negater;

    if (mTimer < mSquareResolution + 1) {
        drawBlurTexture(this);
    } else {
        ImageEffectLocalUtil::setupDrawTexture();
        ImageEffectLocalUtil::drawTexture(mTexture2, 1, 0, mAlpha, ImageEffectLocalUtil::TexDrawType_2);
    }
}

void PauseBlur::calcAnim() {
}

PauseBlur::~PauseBlur() {
}

namespace MR {
    void incrementPauseBlurDelay() {
        if (MR::isExistSceneObj(SceneObj_PauseBlur)) {
            PauseBlur* pBlur = (PauseBlur*)MR::getSceneObjHolder()->getObj(SceneObj_PauseBlur);
            pBlur->mDelay++;
        }
    }

    void decrementPauseBlurDelay() {
        if (MR::isExistSceneObj(SceneObj_PauseBlur)) {
            decrementPauseBlurDelayCount((PauseBlur*)MR::getSceneObjHolder()->getObj(SceneObj_PauseBlur));
        }
    }

    void requestPauseBlurMovementOn() {
        if (MR::isExistSceneObj(SceneObj_PauseBlur)) {
            MR::requestMovementOn(MR::getSceneObjHolder()->getObj(SceneObj_PauseBlur));
        }
    }
}  // namespace MR
