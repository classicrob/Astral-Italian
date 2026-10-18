#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class BreakModel;
class RockBreakCage : public LiveActor {
public:
    RockBreakCage(const char*);
    virtual ~RockBreakCage();
    virtual void init(const JMapInfoIter&);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual void kill();
    void exeDamage();
    s32 mHitPoints;
    BreakModel* mBreakModel;
    const char* mObjectName;
};
