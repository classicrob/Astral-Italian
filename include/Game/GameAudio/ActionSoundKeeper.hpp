#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class AudSoundObject;
class ActionSound;
class ActionSoundInfo;

class ActionSoundKeeper {
public:
    ActionSoundKeeper(s32, const char*, TVec3f*);
    void update();
    void startSound(const char*, s32, s32);

    AudSoundObject* mSoundObject;
    ActionSoundInfo* mInfo;
    ActionSound** mSounds;
    s32 mSoundCount;
};
