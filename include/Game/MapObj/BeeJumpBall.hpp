#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class BeeJumpBall : public LiveActor {
public:
    BeeJumpBall(const char*);
    virtual ~BeeJumpBall();
    virtual void init(const JMapInfoIter&);
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    void exeWait();
    void exeBound();
    void exeBreak();
    void exeRecover();
};
