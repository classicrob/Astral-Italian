#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class GravityLightRoad : public LiveActor {
public:
    GravityLightRoad(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void updateHitSensor(HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeAppear();
    void exeWait();
    void exePlayerFall();
    void exeDisappear();
    void endPlayerFall();
    bool tryPlayerFall();
    bool isPlayerFall() const;
    bool isPlayerInVolume();

    /* 0x90 */ TQuat4f mQuat;
    /* 0xA0 */ TVec3f mFallVec;
    /* 0xAC */ f32 mFallSpeed;
    /* 0xB0 */ f32 mRadius;
    /* 0xB4 */ TVec3f mEndPos;
};
