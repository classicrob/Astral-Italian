#pragma once

#include <revolution.h>

class ActionSoundInfo;
class ActionSoundParam;
class AudSoundObject;

// One sound slot of an ActionSoundKeeper. Layout from ActionSound::ctor (fn_8021F950):
// 0x00 info, 0x04 param block (created in fn_8021F970), 0x08 AudSoundObject, 0x0C f32, 0x10 bool. Size 0x14.
class ActionSound {
public:
    ActionSound(const ActionSoundInfo* pInfo, AudSoundObject* pSoundObject);

    s32 getId() const;
    const char* getName() const;
    bool play(s32 mode, s32 pitch, s32 velocity, s32 volume);

    const ActionSoundInfo* mInfo;
    ActionSoundParam* mParam;
    AudSoundObject* mSoundObject;
    f32 mUnkC;
    bool mUnk10;
};
