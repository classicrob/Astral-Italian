#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class RepeatTimerSwitchingBlock : public LiveActor {
public:
    RepeatTimerSwitchingBlock(const char* pName);

    virtual ~RepeatTimerSwitchingBlock();
    virtual void init(const JMapInfoIter& rIter);
    virtual void control();

    void exeChange();
    void exeWait();
    void exeSwitchOn();
    void exeSwitchOff();
    bool isSwitchBOn();
    s32 isSwitchTimingReady();

    /* 0x90 */ s32 mTimer;
    /* 0x94 */ f32 mWaitStep;
    /* 0x98 */ f32 mWaitRandom;
    /* 0x9C */ f32 mShiftFrame;
    /* 0xA0 */ bool mIsActing;
    /* 0xA1 */ bool mIsInverse;
    /* 0xA2 */ bool mIsQuick;
    /* 0xA4 */ s32 mFrameCount;
    /* 0xA8 */ bool mIsDelayWait;
};
