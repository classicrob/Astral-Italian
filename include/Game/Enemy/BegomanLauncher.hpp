#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class BegomanBase;

class BegomanLauncher : public LiveActor {
public:
    BegomanLauncher(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void kill();

    void onSwitchAppear();
    void exeWaitAfterSwitchOn();
    void exeWait();
    void exeWaitForLaunch();
    void exeLaunch();
    void exeAppear();
    void exeDisappear();

    bool fn_801631C0(const LiveActor*) const;
    LiveActor* fn_80163200(s32) const;

    /* 0x90 */ s32 mBegomanCount;
    /* 0x94 */ s32 mAppearDelay;
    /* 0x98 */ BegomanBase** mBegomanArray;
    /* 0x9C */ s32 mLaunchType;
};
