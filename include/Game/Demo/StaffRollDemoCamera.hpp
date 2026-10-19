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

    TVec3f calcCameraPos() const;  // fn_801441D0
    const char* getCameraName() const;
    void startTurnAround();  // fn_80144300
    void updateTurnAround();  // fn_801443F0
    void startFromCurrentCamera();  // fn_80144490
    void updateSpeedUp();  // fn_80144550
    void updateSlowDown();  // fn_80144630

    LiveActor* mHost;
    ActorCameraInfo* mCameraInfo;
    s32 mMode;
    TVec3f mDirection;
    TVec3f mTarget;
    TVec3f mUp;
    TVec3f mSide;
    f32 mFrame;
};
