#pragma once

#include "Game/AreaObj/AreaObj.hpp"

class ActionSoundKeeper;

class SoundPlayArea : public AreaObj {
public:
    SoundPlayArea(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void movement();

    void setupSoundPos();

    /* 0x48 */ TVec3f mCenter;
    /* 0x54 */ bool _54;
    /* 0x55 */ bool _55;
    /* 0x58 */ ActionSoundKeeper* mSoundObj;
};
