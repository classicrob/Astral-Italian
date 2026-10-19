#include "Game/Screen/PostFog.hpp"
#include "Game/Screen/PostOutline.hpp"
#include "Game/Screen/ImageEffectState.hpp"
#include "Game/Util.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/BloomEffect.hpp"
#include "Game/Screen/BloomEffectSimple.hpp"
#include "Game/Screen/DepthOfFieldBlur.hpp"
#include "Game/Screen/ImageEffectBase.hpp"
#include "Game/Screen/ImageEffectDirector.hpp"
#include "Game/Screen/ScreenBlurEffect.hpp"

void ImageEffectState::update() {
    ImageEffectBase* pCurrent = mHost->mCurrentEffect;
    bool isCurrent = pCurrent == getEffect();

    if (isCurrent) {
        if (getEffect() == nullptr) {
            return;
        }

        if (getEffect()->mIsTurnedOn) {
            return;
        }

        ImageEffectBase* pBase = getEffect();
        pBase->mIsTurnedOn = true;
        pBase->notifyTurnOn();
        return;
    }

    ImageEffectBase* pBase = mHost->mCurrentEffect;

    if (pBase != nullptr) {
        if (pBase->mIsTurnedOn) {
            pBase->mIsTurnedOn = false;
            pBase->notifyTurnOff();
            return;
        }

        if (pBase->isSomething()) {
            return;
        }
    }

    if (getEffect() != nullptr) {
        ImageEffectBase* pBase = getEffect();
        pBase->mIsTurnedOn = true;
        pBase->notifyTurnOn();
    }

    ImageEffectDirector* pHost = mHost;
    pHost->setCurrentEffect(getEffect());
}

void ImageEffectState::forceOff() {
    ImageEffectBase* pBase;

    if (getEffect() == nullptr) {
        return;
    }

    pBase = getEffect();
    pBase->mIsVisible = false;
    pBase->mIsTurnedOn = false;
    pBase->mFadeRate = 0.0f;
    pBase->notifyForceOff();
}

namespace ImageEffectStateImpl {
    StateBloomNormal::StateBloomNormal(ImageEffectDirector* pHost)
        : ImageEffectState(pHost), mSnapToTarget(true), _C(), mCurBloomIntensity(), mBloomIntensity(), mCurThreshold(), mThreshold(), mCurIntensity1(), mIntensity1(), mCurIntensity2(), mIntensity2() {
    }

    void StateBloomNormal::update() {
        if (mSnapToTarget) {
            mCurBloomIntensity = mBloomIntensity;
            mCurThreshold = mThreshold;
            mCurIntensity1 = mIntensity1;
            mCurIntensity2 = mIntensity2;
            mSnapToTarget = false;
        } else {
            f32 rate = 0.1f;
            mCurBloomIntensity += ((mBloomIntensity + 0.5f) - mCurBloomIntensity) * rate;
            mCurThreshold += ((mThreshold + 0.5f) - mCurThreshold) * rate;
            mCurIntensity1 += ((mIntensity1 + 0.5f) - mCurIntensity1) * rate;
            mCurIntensity2 += ((mIntensity2 + 0.5f) - mCurIntensity2) * rate;

            if (mCurBloomIntensity > 255.0f) {
                mCurBloomIntensity = 255.0f;
            }

            if (mCurThreshold > 255.0f) {
                mCurThreshold = 255.0f;
            }

            if (mCurIntensity1 > 255.0f) {
                mCurIntensity1 = 255.0f;
            }

            if (mCurIntensity2 > 255.0f) {
                mCurIntensity2 = 255.0f;
            }
        }

        u8 bloomIntensity = mCurBloomIntensity;
        static_cast< BloomEffect* >(StateBloomNormal::getEffect())->mBloomIntensity = bloomIntensity;

        u8 threshold = mCurThreshold;
        static_cast< BloomEffect* >(StateBloomNormal::getEffect())->mThreshold = threshold;

        u8 intensity1 = mCurIntensity1;
        static_cast< BloomEffect* >(StateBloomNormal::getEffect())->mBlurIntensity1 = intensity1 / 255.0f;

        u8 intensity2 = mCurIntensity2;
        static_cast< BloomEffect* >(StateBloomNormal::getEffect())->mBlurIntensity2 = intensity2 / 255.0f;

        ImageEffectState::update();
    }

    bool StateBloomNormal::doesEffectExist() const {
        return MR::isExistSceneObj(SceneObj_BloomEffect);
    }

    ImageEffectBase* StateBloomNormal::getEffect() const {
        return MR::getSceneObj< ImageEffectBase >(SceneObj_BloomEffect);
    }

    void StateBloomNormal::onChange() {
        mSnapToTarget = true;
    }

    void StateBloomNormal::setBloomIntensity(u8 bloomIntensity) {
        mBloomIntensity = bloomIntensity;
    }

    void StateBloomNormal::setThreshold(u8 threshold) {
        mThreshold = threshold;
    }

    void StateBloomNormal::setIntensity1(u8 intensity1) {
        mIntensity1 = intensity1;
    }

    void StateBloomNormal::setIntensity1Default() {
        mIntensity1 = static_cast< BloomEffect* >(StateBloomNormal::getEffect())->getIntensity1Default();
    }

    void StateBloomNormal::setIntensity2(u8 intensity2) {
        mIntensity2 = intensity2;
    }

    void StateBloomNormal::setIntensity2Default() {
        mIntensity2 = static_cast< BloomEffect* >(StateBloomNormal::getEffect())->getIntensity2Default();
    }

    bool StateBloomSimple::doesEffectExist() const {
        return MR::isExistSceneObj(SceneObj_BloomEffectSimple);
    }

    ImageEffectBase* StateBloomSimple::getEffect() const {
        return MR::getSceneObj< ImageEffectBase >(SceneObj_BloomEffectSimple);
    }

    void StateBloomSimple::setMaskFilterAll() {
        static_cast< BloomEffectSimple* >(StateBloomSimple::getEffect())->mMaskFilter = 0;
    }

    void StateBloomSimple::setMaskFilterRed() {
        static_cast< BloomEffectSimple* >(StateBloomSimple::getEffect())->mMaskFilter = 1;
    }

    void StateBloomSimple::setMaskFilterGreen() {
        static_cast< BloomEffectSimple* >(StateBloomSimple::getEffect())->mMaskFilter = 2;
    }

    void StateBloomSimple::setMaskFilterBlue() {
        static_cast< BloomEffectSimple* >(StateBloomSimple::getEffect())->mMaskFilter = 3;
    }

    void StateBloomSimple::setThreshold(u8 threshold) {
        static_cast< BloomEffectSimple* >(StateBloomSimple::getEffect())->mThreshold = threshold;
    }

    void StateBloomSimple::setIntensity(f32 intensity) {
        static_cast< BloomEffectSimple* >(StateBloomSimple::getEffect())->mIntensity = intensity;
    }

    bool StateScreenBlur::doesEffectExist() const {
        return MR::isExistSceneObj(SceneObj_ScreenBlurEffect);
    }

    ImageEffectBase* StateScreenBlur::getEffect() const {
        return MR::getSceneObj< ImageEffectBase >(SceneObj_ScreenBlurEffect);
    }

    void StateScreenBlur::setIntensity(f32 intensity) {
        static_cast< ScreenBlurEffect* >(StateScreenBlur::getEffect())->mIntensity = intensity;
    }

    bool StateDepthOfField::doesEffectExist() const {
        return MR::isExistSceneObj(SceneObj_DepthOfFieldBlur);
    }

    ImageEffectBase* StateDepthOfField::getEffect() const {
        return MR::getSceneObj< ImageEffectBase >(SceneObj_DepthOfFieldBlur);
    }

    void StateDepthOfField::setIntensity(f32 intensity) {
        static_cast< DepthOfFieldBlur* >(StateDepthOfField::getEffect())->mIntensity = intensity;
    }

    void StateDepthOfField::setBlurMaxDist(s32 blurMaxDist) {
        static_cast< DepthOfFieldBlur* >(StateDepthOfField::getEffect())->mBlurMaxDist = blurMaxDist;
    }

    void StateDepthOfField::setBlurMinDist(s32 blurMinDist) {
        static_cast< DepthOfFieldBlur* >(StateDepthOfField::getEffect())->mBlurMinDist = blurMinDist;
    }
};  // namespace ImageEffectStateImpl

namespace ImageEffectStateImpl {
    void StateDepthOfField::setFogTextureIndex(s32 index) {
        static_cast<DepthOfFieldBlur*>(StateDepthOfField::getEffect())->mFogTextureIndex = index;
    }
    bool StatePostFog::doesEffectExist() const { return MR::isExistSceneObj(SceneObj_PostFog); }
    ImageEffectBase* StatePostFog::getEffect() const { return static_cast<PostFog*>(MR::getSceneObjHolder()->getObj(SceneObj_PostFog)); }
    void StatePostFog::setMinRange(u16 value) { static_cast<PostFog*>(StatePostFog::getEffect())->mStartDepth = value; }
    void StatePostFog::setMaxRange(u16 value) { static_cast<PostFog*>(StatePostFog::getEffect())->mEndDepth = value; }
    void StatePostFog::setAlpha(u8 value) { static_cast<PostFog*>(StatePostFog::getEffect())->mAlpha = value; }
    void StatePostFog::setRed(u8 value) { static_cast<PostFog*>(StatePostFog::getEffect())->mRed = value; }
    void StatePostFog::setGreen(u8 value) { static_cast<PostFog*>(StatePostFog::getEffect())->mGreen = value; }
    void StatePostFog::setBlue(u8 value) { static_cast<PostFog*>(StatePostFog::getEffect())->mBlue = value; }
    void StatePostFog::setGradType(s32 value) { static_cast<PostFog*>(StatePostFog::getEffect())->mFogTextureIndex = value; }
    bool StatePostOutline::doesEffectExist() const { return MR::isExistSceneObj(SceneObj_PostOutline); }
    ImageEffectBase* StatePostOutline::getEffect() const { return static_cast<PostOutline*>(MR::getSceneObjHolder()->getObj(SceneObj_PostOutline)); }
    void StatePostOutline::setIntensity(u8 value) { static_cast<PostOutline*>(StatePostOutline::getEffect())->mThreshold = value; }
}
