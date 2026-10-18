#pragma once

#include "Game/GameAudio/ActionSoundInfo.hpp"
#include <revolution.h>

class ActorSoundHolder {
public:
    /* 0x00 */ u8 _0[4];
    /* 0x04 */ ActionSoundInfo* mActionSoundInfo;

    void update();  // fn name from Util/SoundUtil usage (0x8022CE00)
};
