#pragma once

#include "Game/Enemy/ActorActionSoundKeeper.hpp"
#include <revolution.h>

class AudAnmSoundObject;
class LiveActor;
#include <JSystem/JGeometry/TVec.hpp>

class ActorSoundHolder {
public:
    ActorSoundHolder(LiveActor*, s32, const char*, const TVec3f*, const TVec3f&);
    /* 0x00 */ AudAnmSoundObject* mSoundObject;
    /* 0x04 */ ActorActionSoundKeeper* mActionSoundKeeper;

    /* 0x08 */ TVec3f* mPosition;
    /* 0x0C */ MtxPtr mPositionMtx;
    /* 0x10 */ const TVec3f* mLocalPosition;

    bool setSoundPositionPtr(TVec3f*);

    void resetSyncBck();
    void resetSyncBckFrame();
    void startAction(const char*);
    void update();  // fn name from Util/SoundUtil usage (0x8022CE00)
    AudAnmSoundObject* getSoundObj();  // asm 0x8022CD70 (ActorSoundHolder.cpp): returns mSoundObj at 0x00
};
