#pragma once
#include "Game/NameObj/NameObj.hpp"
class ActionSoundData;
class ActionSoundSystem : public NameObj {
public:
    ActionSoundSystem(const char*);
    virtual ~ActionSoundSystem();
    ActionSoundData* fn_80220970(const char*);
    ActionSoundData** mData;
    s32 mCount;
};
