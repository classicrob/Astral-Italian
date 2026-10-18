#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class JumpStandAuto : public LiveActor {
public:
    JumpStandAuto(const char*);
    virtual ~JumpStandAuto();
    virtual void init(const JMapInfoIter&);
    virtual void attackSensor(HitSensor*, HitSensor*);
    void exeWait();
    void exeExtend();
    void jumpPlayer(f32);
};
