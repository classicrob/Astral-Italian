#pragma once

#include "Game/AreaObj/AreaObj.hpp"

class AudioEffectArea : public AreaObj {
public:
    AudioEffectArea(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void movement();

    virtual const char* getManagerName() const {
        return "AudioEffectArea";
    }

    /* 0x3C */ s32 mEffectType;
    /* 0x40 */ s32 _40;
};