#pragma once

#include <revolution/types.h>

class JAISoundID;

namespace AudStageBgmWrap {
    u32 changeStageNameToSoundID(const char*, const char*, s32);
    u32 getStageBgmIDAtIndex(const char*, s32, s32);  // was fn_8008A150
    s32 getStageBgmVolumeIDAtIndex(const char*, s32);  // was fn_8008A1A0
    JAISoundID getCometEventBgm(const char*);
    JAISoundID getKoopaFortressAppearBgm(const char*);
};  // namespace AudStageBgmWrap
