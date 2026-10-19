#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class AudSoundObject;
class ActionSound;
class ActionSoundData;
class ActionSoundInfo;

class ActionSoundKeeper {
public:
    ActionSoundKeeper(s32, const char*, TVec3f*);
    void update();
    s32 startLevelSound(const char*, s32, s32, s32);
    ActionSoundInfo* getSoundInfo(const char*) const;
    void startSound(const char*, s32, s32);
    void startSoundAuto(const char*, s32, s32);
    s32 playSounds(s32 mode, const char* pName, s32 pitch, s32 velocity, s32 volume);

    AudSoundObject* mSoundObject;
    ActionSoundData* mInfo;
    ActionSound** mSounds;
    s32 mSoundCount;
};
