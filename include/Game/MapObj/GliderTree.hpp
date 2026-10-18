#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class GliderTree : public LiveActor {
public:
    GliderTree(const char*);
    virtual ~GliderTree();
    virtual void init(const JMapInfoIter&);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    void exeSwing();
};
