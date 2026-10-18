#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class ClipAreaDrop;
class ScatterFireBallLava : public LiveActor {
public:
    ScatterFireBallLava(LiveActor*, const char*);
    virtual ~ScatterFireBallLava();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void kill();
    virtual void attackSensor(HitSensor*, HitSensor*);
    void fn_8017F450(const TVec3f&);
    void fn_8017F540();
    void fn_8017F5C0();
    void exeWait();
    void exeHold();
    /* 0x90 */ LiveActor* mHost;
    /* 0x94 */ ClipAreaDrop* mClipArea;
    /* 0x98 */ f32 mRadius;
};
