#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class ConstructionLight : public LiveActor {
public:
    ConstructionLight(const char*);
    virtual ~ConstructionLight();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    void exeNormal();
    void exeBreak();
    bool mDropsCoin;
};
