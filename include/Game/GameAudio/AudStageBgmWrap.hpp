#pragma once

#include <revolution/types.h>

class JAISoundID;

namespace AudStageBgmWrap {
    JAISoundID changeStageNameToSoundID(const char*, const char*, s32);
    JAISoundID getStageBgmIDAtIndex(const char*, s32, s32);  // PLACEHOLDER(core): fn_8008A150, name unknown
    s32 getStageBgmVolumeIDAtIndex(const char*, s32);  // PLACEHOLDER(core): fn_8008A1A0, name unknown
    JAISoundID getCometEventBgm(const char*);
    JAISoundID getKoopaFortressAppearBgm(const char*);
};  // namespace AudStageBgmWrap
