#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class BindCone;
class ActorCameraInfo;

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
    bool isNerveTypeNotWait(s32);

    /* 0x90 */ BindCone* mCone;
    /* 0x94 */ HitSensor* mBoundSensor;
    /* 0x98 */ ActorCameraInfo* mCameraInfo;
    /* 0x9C */ TPos3f mBaseMtx;
    /* 0xCC */ f32 mLaunchTime;
    /* 0xD0 */ TVec3f mLaunchStart;
    /* 0xDC */ TVec3f mBindVelocity;
    /* 0xE8 */ TVec3f mHoleGravity;
    /* 0xF4 */ TVec3f mLaunchGravity;
    /* 0x100 */ TVec3f mLaunchDirection;
    /* 0x10C */ u32 mLaunchDuration;
    /* 0x110 */ f32 mLaunchDistance;
    /* 0x114 */ f32 mLaunchQuadratic;
    /* 0x118 */ f32 mLaunchLinear;
    /* 0x11C */ s32 mSettledFrames;
};
