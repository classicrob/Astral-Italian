#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class RockReflector : public LiveActor {
public:
    RockReflector(const char*);
    virtual ~RockReflector();
    virtual void init(const JMapInfoIter&);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    bool isNerveWait() const NO_INLINE;
    void exeReflect();
};
