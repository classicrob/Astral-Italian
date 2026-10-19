#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class JAISoundID;

class SoundEmitter : public LiveActor {
public:
    SoundEmitter(const char*);

    virtual ~SoundEmitter();
    virtual void init(const JMapInfoIter&);
    virtual void control();

    void exePlaySound();
    void exeStandBy();

    /* 0x90 */ s32 mSoundIndex;
    /* 0x94 */ s32 mRailTargetMode;
    /* 0x98 */ bool mHasDemo;
    /* 0x99 */ bool mIsStoryBookDemo;
    /* 0x9A */ bool mIsRepeat;
};
