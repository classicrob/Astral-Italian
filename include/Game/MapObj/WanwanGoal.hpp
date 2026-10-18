#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class WanwanGoal : public LiveActor {
public:
    WanwanGoal(const char*);
    virtual ~WanwanGoal();
    virtual void init(const JMapInfoIter&);
    virtual void attackSensor(HitSensor*, HitSensor*);
    bool mIsReached;
};
