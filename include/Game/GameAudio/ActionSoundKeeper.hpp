#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class AudSoundObject;
class ActionSound;
class ActionSoundInfo;

class ActionSoundKeeper {
public:
    ActionSoundKeeper(s32, const char*, TVec3f*);
    void update();

    AudSoundObject* mSoundObject;
    ActionSoundInfo* mInfo;
    ActionSound** mSounds;
    s32 mSoundCount;
};
