#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class TubeSliderJumpStep : public LiveActor {
public:
    TubeSliderJumpStep(const TVec3f&);
    virtual ~TubeSliderJumpStep();
    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    void exeWait();
    void exeJump();
    TVec3f mFront;
};
