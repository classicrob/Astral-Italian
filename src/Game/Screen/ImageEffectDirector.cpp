#include "Game/Screen/ImageEffectDirector.hpp"
#include "Game/AreaObj/BloomArea.hpp"
#include "Game/AreaObj/DepthOfFieldArea.hpp"
#include "Game/AreaObj/ScreenBlurArea.hpp"
#include "Game/AreaObj/PostFogArea.hpp"
#include "Game/AreaObj/PostOutlineArea.hpp"
#include "Game/Camera/CameraDirector.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/BloomEffect.hpp"
#include "Game/Screen/BloomEffectSimple.hpp"
#include "Game/Screen/DepthOfFieldBlur.hpp"
#include "Game/Screen/ImageEffectBase.hpp"
#include "Game/Screen/ImageEffectLocalUtil.hpp"
#include "Game/Screen/ImageEffectState.hpp"
#include "Game/Util.hpp"

namespace {
    static const s32 sIntensityCountMax = 60;
    static const f32 sDOFIntensity = 1.0f;
};  // namespace

ImageEffectDirector::ImageEffectDirector(const char* pName)
    : NameObj(pName), mIsAuto(true), mIsPlayerSync(), mWasDOFSuppressed(), mIsDOFSuppressed(), mPlayerSyncIntensity(), mDepthOfFieldIntensity(::sDOFIntensity),
      mStateNull(new ImageEffectStateImpl::StateNull(this)), mStateBloomNormal(new ImageEffectStateImpl::StateBloomNormal(this)),
      mStateScreenBlur(new ImageEffectStateImpl::StateScreenBlur(this)),
      mStateDepthOfField(new ImageEffectStateImpl::StateDepthOfField(this)),
      mStatePostFog(new ImageEffectStateImpl::StatePostFog(this)), mStatePostOutline(new ImageEffectStateImpl::StatePostOutline(this)), mState(mStateNull), mCurrentEffect() {
    MR::connectToSceneImageEffectMovement(this);
}

void ImageEffectDirector::movement() {
    if (mIsAuto) {
        updateAuto();
    } else {
        updateManual();
    }

    bool isDOF = mIsDOFSuppressed && mState == mStateDepthOfField;

    if (mWasDOFSuppressed) {
        if (!isDOF) {
            mState->onChange();
        }
    } else if (isDOF) {
        mState->forceOff();
        mStateNull->onChange();
    }

    if (isDOF) {
        mStateNull->update();
    } else {
        mState->update();
    }

    mWasDOFSuppressed = isDOF;
}

void ImageEffectDirector::setCurrentEffect(ImageEffectBase* pEffect) {
    if (mCurrentEffect != pEffect) {
        mCurrentEffect = pEffect;
    }
}

void ImageEffectDirector::turnOnNormal() {
    mIsAuto = false;
    setState(mStateBloomNormal);
}

void ImageEffectDirector::turnOnDepthOfField(bool p1) {
    mIsAuto = false;
    setState(mStateDepthOfField);
    setPlayerSync(p1);
    mDepthOfFieldIntensity = ::sDOFIntensity;
    mStateDepthOfField->setBlurMinDist(-1);
    mStateDepthOfField->setBlurMaxDist(-1);
}

void ImageEffectDirector::turnOff() {
    mIsAuto = false;
    setState(mStateNull);
}

void ImageEffectDirector::forceOff() {
    mIsAuto = false;
    mState->forceOff();
    setState(mStateNull);
}

void ImageEffectDirector::setAuto() {
    mIsAuto = true;
}

void ImageEffectDirector::turnOffDOFInSubjective() {
    mIsDOFSuppressed = true;
}

void ImageEffectDirector::turnOnDOFInSubjective() {
    mIsDOFSuppressed = false;
}

void ImageEffectDirector::setNormalBloomIntensity(u8 bloomIntensity) {
    mStateBloomNormal->setBloomIntensity(bloomIntensity);
}

void ImageEffectDirector::setNormalBloomThreshold(u8 threshold) {
    mStateBloomNormal->setThreshold(threshold);
}

void ImageEffectDirector::setNormalBloomBlurIntensity1(u8 intensity1) {
    mStateBloomNormal->setIntensity1(intensity1);
}

void ImageEffectDirector::setNormalBloomBlurIntensity2(u8 intensity2) {
    mStateBloomNormal->setIntensity2(intensity2);
}

void ImageEffectDirector::setDepthOfFieldIntensity(f32 intensity) {
    mDepthOfFieldIntensity = intensity;
}

void ImageEffectDirector::updateManual() {
    if (mIsPlayerSync) {
        updateSyncCounter();
    } else {
        incPlayerSyncIntensity();
    }

    f32 intensity = mDepthOfFieldIntensity * (mPlayerSyncIntensity / 60.0f);

    if (mState == mStateScreenBlur) {
        mStateScreenBlur->setIntensity(intensity);
    } else if (mState == mStateDepthOfField) {
        mStateDepthOfField->setIntensity(intensity);
    }
}

void ImageEffectDirector::setState(ImageEffectState* pState) {
    if (mState != pState) {
        mState = pState;
        mState->onChange();
    }
}

void ImageEffectDirector::setBloomNormalParams(ImageEffectArea* pArea) {
    BloomArea* pBloomArea = static_cast< BloomArea* >(pArea);
    mStateBloomNormal->setBloomIntensity(pBloomArea->mIntensity);
    mStateBloomNormal->setThreshold(pBloomArea->mThreshold);

    if (pBloomArea->mIntensity1 >= 0) {
        mStateBloomNormal->setIntensity1(pBloomArea->mIntensity1);
    } else {
        mStateBloomNormal->setIntensity1Default();
    }

    if (pBloomArea->mIntensity2 >= 0) {
        mStateBloomNormal->setIntensity2(pBloomArea->mIntensity2);
    } else {
        mStateBloomNormal->setIntensity2Default();
    }
}

// NONMATCHING: equivalent integer argument register scheduling.
void ImageEffectDirector::setPostFogParams(ImageEffectArea* area) {
    PostFogArea* fog = static_cast<PostFogArea*>(area);
    mStatePostFog->setMinRange(fog->getMinRange());
    mStatePostFog->setMaxRange(fog->getMaxRange());
    mStatePostFog->setAlpha(fog->getAlpha());
    mStatePostFog->setRed(fog->getRed());
    mStatePostFog->setGreen(fog->getGreen());
    mStatePostFog->setBlue(fog->getBlue());
    mStatePostFog->setGradType(fog->getGradType());
}

void ImageEffectDirector::setDepthOfFieldBlurDistance(s32 minDist, s32 maxDist) {
    mStateDepthOfField->setBlurMinDist(minDist);
    mStateDepthOfField->setBlurMaxDist(maxDist);
}

void ImageEffectDirector::setScreenBlurParams(ImageEffectArea* pArea) {
    ScreenBlurArea* pScreenBlurArea = static_cast< ScreenBlurArea* >(pArea);
    updatePlayerSyncIntensity(pScreenBlurArea, mStateScreenBlur);
    mStateScreenBlur->setIntensity(mPlayerSyncIntensity / 60.0f);
}

void ImageEffectDirector::setDepthOfFieldParams(ImageEffectArea* pArea) {
    DepthOfFieldArea* pDepthOfFieldArea = static_cast< DepthOfFieldArea* >(pArea);
    updatePlayerSyncIntensity(pDepthOfFieldArea, mStateDepthOfField);
    f32 syncIntensity = mPlayerSyncIntensity / 60.0f;
    f32 areaIntensity = pDepthOfFieldArea->getIntensity();
    mStateDepthOfField->setIntensity(syncIntensity * areaIntensity);
    mStateDepthOfField->setBlurMaxDist(pDepthOfFieldArea->getBlurMaxDist());
    mStateDepthOfField->setBlurMinDist(pDepthOfFieldArea->getBlurMinDist());
    mStateDepthOfField->setFogTextureIndex(pDepthOfFieldArea->getArg4());
}

void ImageEffectDirector::initPlayerSyncIntensity(ImageEffectArea* pArea, ImageEffectState* pState) {
    if (mState != pState) {
        if (pArea->isSyncWithPlayer()) {
            mPlayerSyncIntensity = 0;
        } else {
            mPlayerSyncIntensity = ::sIntensityCountMax;
        }
    }
}

void ImageEffectDirector::updatePlayerSyncIntensity(ImageEffectArea* pArea, ImageEffectState* pState) {
    initPlayerSyncIntensity(pArea, pState);

    if (pArea->isSyncWithPlayer()) {
        updateSyncCounter();
    } else {
        incPlayerSyncIntensity();
    }
}

void ImageEffectDirector::updateSyncCounter() {
    if (MR::getCameraDirector()->isRotatingHard()) {
        incPlayerSyncIntensity();
    } else {
        decPlayerSyncIntensity();
    }
}

void ImageEffectDirector::incPlayerSyncIntensity() {
    mPlayerSyncIntensity++;

    if (mPlayerSyncIntensity > ::sIntensityCountMax) {
        mPlayerSyncIntensity = ::sIntensityCountMax;
    }
}

void ImageEffectDirector::decPlayerSyncIntensity() {
    mPlayerSyncIntensity--;

    if (mPlayerSyncIntensity < 0) {
        mPlayerSyncIntensity = 0;
    }
}

void ImageEffectDirector::setPlayerSync(bool isPlayerSync) {
    mIsPlayerSync = isPlayerSync;

    if (isPlayerSync) {
        mPlayerSyncIntensity = 0;
    } else {
        mPlayerSyncIntensity = ::sIntensityCountMax;
    }
}

// NONMATCHING: equivalent outline intensity argument register scheduling.
// NONMATCHING: equivalent narrowed argument and state-pointer loads schedule differently.
void ImageEffectDirector::updateAuto() {
    AreaObj* pArea = MR::getAreaObj("ImageEffectArea", *MR::getPlayerPos());

    if (pArea == nullptr) {
        setState(mStateNull);
        return;
    }

    ImageEffectArea* pEffectArea = static_cast< ImageEffectArea* >(pArea);
    switch (pEffectArea->mEffectType) {
    case ImageEffectArea::IMAGE_EFFECT_TYPE_BLOOM:
        setBloomNormalParams(pEffectArea);
        setState(mStateBloomNormal);
        break;
    case ImageEffectArea::IMAGE_EFFECT_TYPE_BLUR:
        setScreenBlurParams(pEffectArea);
        setState(mStateScreenBlur);
        break;
    case ImageEffectArea::IMAGE_EFFECT_TYPE_DOF:
        setDepthOfFieldParams(pEffectArea);
        setState(mStateDepthOfField);
        break;
    case ImageEffectArea::IMAGE_EFFECT_TYPE_POST_FOG:
        setPostFogParams(pEffectArea);
        setState(mStatePostFog);
        break;
    case ImageEffectArea::IMAGE_EFFECT_TYPE_POST_OUTLINE:
        mStatePostOutline->setIntensity(static_cast<PostOutlineArea*>(pEffectArea)->getIntensity());
        setState(mStatePostOutline);
        break;
    default:
        setState(mStateNull);
        break;
    }
}
