#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class DashRing : public LiveActor {
public:
    DashRing(const char* pName);

    virtual ~DashRing();
    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void draw() const;
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);

    void calcAxis();
    void calcSubAxis();
    void updateAxisTowardsPlayer();

    /* 0x90 */ TVec3f mClippingPos;
    /* 0x9C */ TVec3f mAxis;
    /* 0xA8 */ TVec3f mSubAxis;
    /* 0xB4 */ u16 mActiveTime;
    /* 0xB6 */ u16 mRevivalTime;
    /* 0xB8 */ s32 mBoostTime;
    /* 0xBC */ s32 mMaxDuration;
    /* 0xC0 */ f32 mSpeedScale;
    /* 0xC4 */ bool mDoTurn;
    /* 0xC8 */ TVec3f mInitialPosition;
};
