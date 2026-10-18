#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry.hpp>
class JointController;
class JointControllerInfo;
class BigFanCloud : public LiveActor {
public:
    BigFanCloud(const char*);
    virtual ~BigFanCloud();
    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();
    virtual void control();
    void exeWait();
    void exeStart();
    void exeMove();
    void exeStop();
    bool calcJoint(TPos3f*, const JointControllerInfo&);
    bool tryStart();
    bool tryStop();
    JointController* mJointController;
    f32 mAngle;
    f32 mSpeed;
};
