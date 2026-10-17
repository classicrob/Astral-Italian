#pragma once

#include <JSystem/JGeometry.hpp>
#include <revolution.h>

class NameObj;
class LiveActor;

class SpinHitController {
public:
    SpinHitController(const LiveActor*);
    SpinHitController(const LiveActor*, s32, s32, f32, f32, f32);

    void setParameter(const LiveActor*, s32, s32, f32, f32, f32);
    void start(LiveActor*, const TVec3f&, const TVec3f&);
    void startWithoutStopScene(LiveActor*, const TVec3f&, const TVec3f&);
    bool execute(LiveActor*);

    /* 0x00 */ s32 mKillStep;
    /* 0x04 */ s32 mStopSceneFrame;
    /* 0x08 */ f32 mGravityAccel;
    /* 0x0C */ f32 mHorizontalSpeed;
    /* 0x10 */ f32 mUpSpeed;
    /* 0x14 */ bool mIsStopScene;
};
