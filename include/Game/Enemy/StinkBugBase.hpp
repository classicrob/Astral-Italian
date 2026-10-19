#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class HitSensor;

class StinkBugBase : public LiveActor {
public:
    StinkBugBase(const char*);

    virtual void init(const JMapInfoIter&);

    void setDashVelocity(f32);
    bool tryTurnSearch(f32);
    bool tryTurnDashSign(f32);
    void fixInitPos();
    bool isPlayerInTerritory(f32, f32, f32, f32) const;
    bool isHitHorn(HitSensor*, HitSensor*, f32) const;

    /* 0x8C */ TVec3f mFront;
    /* 0x98 */ TVec3f mHomePos;
    /* 0xA4 */ TVec3f mHomeFront;
    /* 0xB0 */ f32 mMaxTurnDegree;
    /* 0xB4 */ f32 mRadius;
    /* 0xB8 */ bool mTurnPositive;
};
