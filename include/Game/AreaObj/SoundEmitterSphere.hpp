#pragma once

#include "Game/AreaObj/AreaObj.hpp"

class ActionSoundKeeper;

class SoundEmitterSphere : public AreaObj {
public:
    SoundEmitterSphere(int formType, const char* pName);

    virtual ~SoundEmitterSphere();
    virtual void init(const JMapInfoIter& rIter);
    virtual void movement();

    /* 0x48 */ ActionSoundKeeper* mSoundObj;
    /* 0x4C */ TVec3f mSoundPos;
};
