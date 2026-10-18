#pragma once
#include <JSystem/JGeometry/TVec.hpp>
class LiveActor;
class ActorCameraInfo;
class StaffRollDemoCamera {
public:
    StaffRollDemoCamera(LiveActor*, ActorCameraInfo*, s32);
    void start();
    void update(bool);
    void end();
    LiveActor* mHost;
    ActorCameraInfo* mCameraInfo;
    s32 mMode;
    TVec3f mDirection;
    TVec3f mTarget;
    TVec3f mUp;
    TVec3f mSide;
    f32 mFrame;
};
