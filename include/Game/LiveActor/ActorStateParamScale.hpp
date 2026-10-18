#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class JMapInfoIter;
class LiveActor;
class Nerve;

class ActorStateParamScale {
public:
    f32 fn_8022D440(s32);
    f32 fn_8022D3E0(s32);
    ActorStateParamScale(LiveActor* pHost);

    void init(const JMapInfoIter& rIter);
    void init(const ActorStateParamScale* pOther);
    f32 getSpeed();
    f32 getNerveStepRate();
    bool isStep(s32 step);
    bool isFirstStep();
    bool isGreaterStep(s32 step);
    bool isGreaterEqualStep(s32 step);
    bool isIntervalStep(s32 interval, s32 offset);
    void update();
    void resetHostVelocity();
    void calcHostVelocity();
    LiveActor* calcHostVelocity(const TVec3f& rVel);
    void calcHostVelocity(const TVec3f& rVel, const TVec3f& rScale, f32 rate);
    f32 calcStepRatio(s32 maxStep);
    void updateStepRatioEase(s32 maxStep);

private:
    bool isNeedForReset(const Nerve* pNerve, s32 step);

    /* 0x00 */ bool mIsOnSwitch;
    /* 0x01 */ bool mIsUseSwitch;
    /* 0x04 */ f32 mNerveStepRate;
    /* 0x08 */ f32 mPrevNerveStepRate;
    /* 0x0C */ f32 mSpeedOn;
    /* 0x10 */ f32 mSpeedOff;
    /* 0x14 */ TVec3f mSavedVel;
    /* 0x20 */ const Nerve* mNerve;
    /* 0x24 */ s32 mNerveStep;
    /* 0x28 */ LiveActor* mHost;
    /* 0x2C */ LiveActor* mSwitchActor;
};
