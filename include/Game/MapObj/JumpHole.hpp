#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class BindCone;

class JumpHole : public LiveActor {
public:
    JumpHole(const char* pName);

    virtual ~JumpHole() {
    }

    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void control();
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeWait();
    void exeSetCenter();
    void exeSetUp();
    void exeRailMove();
    void bindHole();
    void initParabola(const TVec3f& rPosition);

    /* 0x90 */ BindCone* mCone;
    /* 0x94 */ HitSensor* mBoundSensor;
    /* 0x98 */ f32 mLaunchTime;
    /* 0x9C */ TVec3f mLaunchStart;
    /* 0xA8 */ TVec3f mBindVelocity;
    /* 0xB4 */ TVec3f mHoleGravity;
    /* 0xC0 */ TVec3f mLaunchGravity;
    /* 0xCC */ TVec3f mLaunchDirection;
    /* 0xD8 */ u32 mLaunchDuration;
    /* 0xDC */ f32 mLaunchDistance;
    /* 0xE0 */ f32 mLaunchQuadratic;
    /* 0xE4 */ f32 mLaunchLinear;
    /* 0xE8 */ s32 mSettledFrames;
};
