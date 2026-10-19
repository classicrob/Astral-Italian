#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class SphereRailDash : public LiveActor {
public:
    SphereRailDash(const char*);

    virtual ~SphereRailDash();
    virtual void init(const JMapInfoIter&);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeRailMove();


    /* 0x090 */ HitSensor* mSensor;
    /* 0x094 */ TVec3f mOffset;
    /* 0x0A0 */ f32 mSensorRadius;
    /* 0x0A4 */ f32 mEndSpeed;
    /* 0x0A8 */ s32 mBlendStep;
    /* 0x0AC */ TVec3f _AC;
    /* 0x0B8 */ TVec3f mSideVelocity;
    /* 0x0C4 */ f32 mAccelCubic;
    /* 0x0C8 */ f32 mAccelSquare;
    /* 0x0CC */ f32 mStartSpeed;
    /* 0x0D0 */ s32 mDuration;
};
