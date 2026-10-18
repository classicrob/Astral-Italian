#pragma once

#include "Game/NameObj/NameObj.hpp"
#include <revolution.h>

class JUTTexture;

// Screen blur shown while the game is paused (SMG2 SceneObj_PauseBlur).
class PauseBlur : public NameObj {
public:
    PauseBlur(const char*);
    virtual ~PauseBlur();

    virtual void init(const JMapInfoIter&);
    virtual void movement();
    virtual void draw() const;
    virtual void calcAnim();

    /* 0x14 */ JUTTexture* mTexture1;
    /* 0x18 */ JUTTexture* mTexture2;
    /* 0x1C */ u8 mAlpha;
    /* 0x20 */ s32 mTimer;
    /* 0x24 */ s32 mResolution;
    /* 0x28 */ s32 mHalfResolution;
    /* 0x2C */ s32 mSquareResolution;
    /* 0x30 */ s32 mDelay;
    /* 0x34 */ bool mIsActive;
};
