#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class SupportTicoStop;

class LavaGeyser : public LiveActor {
public:
    LavaGeyser(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void startClipped();
    virtual void kill();
    virtual void control();
    virtual void updateHitSensor(HitSensor*);
    virtual void attackSensor(HitSensor*, HitSensor*);

    void exeWait();
    void exeWaitSwitch();
    void exeSign();
    void exeShootUp();
    void exeShootKeep();
    void exeShootDown();

    /* 0x90 */ SupportTicoStop* mTicoStop;
    /* 0x94 */ s32 mWaitTime;
    /* 0x98 */ s32 mShootKeepTime;
    /* 0x9C */ TVec3f mTopPos;
    /* 0xA8 */ TVec3f mUpDir;
    /* 0xB4 */ f32 _AC;
    /* 0xB8 */ f32 _B0;  // one of these may be a different data type
    /* 0xBC */ f32 _B4;
    /* 0xC0 */ f32 _B8;
};
