#pragma once

#include "Game/NameObj/NameObj.hpp"

class ImageEffectBase : public NameObj {
public:
    /// @brief Creates a new `ImageEffectBase`.
    /// @param pName A pointer to the null-terminated name of the object.
    ImageEffectBase(const char* pName);

    virtual void calcAnim();

    virtual void calcAnimSub() {
    }

    virtual void notifyTurnOn() {
    }

    virtual void notifyTurnOff() {
    }

    virtual void notifyForceOn() {
    }

    virtual void notifyForceOff() {
    }

    bool isSomething() const {
        return mIsTurnedOn || mIsVisible;
    }

    f32 getFadeRate() const {
        return mFadeRate;
    }

    /* 0x0C */ bool mIsTurnedOn;
    /* 0x0D */ bool mIsVisible;
    /* 0x10 */ f32 mFadeRate;
};
