#pragma once

#include "Game/NameObj/NameObj.hpp"

class ImageEffectArea;
class ImageEffectBase;
class ImageEffectState;

namespace ImageEffectStateImpl {
    class StateBloomNormal;
    class StatePostFog;
    class StatePostOutline;
    class StateDepthOfField;
    class StateNull;
    class StateScreenBlur;
};  // namespace ImageEffectStateImpl

class ImageEffectDirector : public NameObj {
public:
    /// @brief Creates a new `ImageEffectDirector`.
    /// @param pName A pointer to the null-terminated name of the object.
    ImageEffectDirector(const char* pName);

    virtual void movement();

    void setCurrentEffect(ImageEffectBase*);
    void turnOnNormal();
    void turnOnDepthOfField(bool);
    void turnOff();
    void forceOff();
    void setAuto();
    void turnOffDOFInSubjective();
    void turnOnDOFInSubjective();
    void setNormalBloomIntensity(u8);
    void setNormalBloomThreshold(u8);
    void setNormalBloomBlurIntensity1(u8);
    void setNormalBloomBlurIntensity2(u8);
    void setDepthOfFieldIntensity(f32);
    void updateManual();
    void setState(ImageEffectState*);
    void setBloomNormalParams(ImageEffectArea*);
    void setPostFogParams(ImageEffectArea*);
    void setDepthOfFieldBlurDistance(s32, s32);
    void setScreenBlurParams(ImageEffectArea*);
    void setDepthOfFieldParams(ImageEffectArea*);
    void initPlayerSyncIntensity(ImageEffectArea*, ImageEffectState*);
    void updatePlayerSyncIntensity(ImageEffectArea*, ImageEffectState*);
    void updateSyncCounter();
    void incPlayerSyncIntensity();
    void decPlayerSyncIntensity();
    void setPlayerSync(bool);
    void updateAuto();

    /* 0x12 */ bool mIsAuto;
    /* 0x13 */ bool mIsPlayerSync;
    /* 0x14 */ bool mWasDOFSuppressed;
    /* 0x15 */ bool mIsDOFSuppressed;
    /* 0x18 */ s32 mPlayerSyncIntensity;
    /* 0x1C */ f32 mDepthOfFieldIntensity;
    /* 0x20 */ ImageEffectStateImpl::StateNull* mStateNull;
    /* 0x24 */ ImageEffectStateImpl::StateBloomNormal* mStateBloomNormal;
    /* 0x28 */ ImageEffectStateImpl::StateScreenBlur* mStateScreenBlur;
    /* 0x2C */ ImageEffectStateImpl::StateDepthOfField* mStateDepthOfField;
    /* 0x30 */ ImageEffectStateImpl::StatePostFog* mStatePostFog;
    /* 0x34 */ ImageEffectStateImpl::StatePostOutline* mStatePostOutline;
    /* 0x38 */ ImageEffectState* mState;
    /* 0x3C */ ImageEffectBase* mCurrentEffect;
};
