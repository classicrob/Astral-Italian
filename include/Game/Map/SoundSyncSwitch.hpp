#pragma once

#include "Game/NameObj/NameObj.hpp"
#include <revolution.h>

class StageSwitchCtrl;
class ActionSoundKeeper;

// Sound-synced stage switch (SMG2 only). Layout is the target's: all fields in 0x14..0x34.
class SoundSyncSwitch : public NameObj {
public:
    SoundSyncSwitch(const char*);

    virtual ~SoundSyncSwitch();
    virtual void init(const JMapInfoIter&);
    virtual void movement();

    void emit();

    /* 0x14 */ StageSwitchCtrl* mStageSwitchCtrl;
    /* 0x18 */ s32 mBeatNum;
    /* 0x1C */ s32 mTimeOffset;
    /* 0x20 */ s32 mInterval;
    /* 0x24 */ s32 mSwitchOnTime;
    /* 0x28 */ s32 mCounter;
    /* 0x2C */ bool mIsCameraWait;
    /* 0x2D */ bool mIsSwitchOnAfterCamera;
    /* 0x30 */ s32 mSoundType;
    /* 0x34 */ ActionSoundKeeper* mSound;
};
