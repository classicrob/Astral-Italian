#pragma once

#include "Game/NameObj/NameObj.hpp"

class AudEffectDirector : public NameObj {
public:
    AudEffectDirector();

    virtual void init(const JMapInfoIter& rIter);
    virtual void movement();

    void setEffectType(s32, s32, s32);
    void restoreEffectType() NO_INLINE;
    void setAudioEffectParam(s32);

    /* 0x14 */ s32 mEffectType;
    /* 0x18 */ s32 mAppliedEffectType;
    /* 0x1C */ s32 mPriority;
    /* 0x20 */ s32 mSavedEffectType;
};
