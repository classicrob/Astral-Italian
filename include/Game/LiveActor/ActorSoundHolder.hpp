#pragma once

#include "Game/Enemy/ActorActionSoundKeeper.hpp"
#include <revolution.h>

class AudAnmSoundObject;
class LiveActor;
#include <JSystem/JGeometry/TVec.hpp>

class ActorSoundHolder {
public:
    ActorSoundHolder(LiveActor*, s32, const char*, const TVec3f*, const TVec3f&);
    /* 0x00 */ u8 _0[4];
    /* 0x04 */ ActorActionSoundKeeper* mActionSoundKeeper;

    /* 0x08 */ u8 mUnk08[0xC];

    void resetSyncBck();
    void resetSyncBckFrame();
    void startAction(const char*);
    void update();  // fn name from Util/SoundUtil usage (0x8022CE00)
    AudAnmSoundObject* getSoundObj();  // asm 0x8022CD70 (ActorSoundHolder.cpp): returns mSoundObj at 0x00
};
