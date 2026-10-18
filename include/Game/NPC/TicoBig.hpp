#pragma once
#include "Game/NPC/NPCActor.hpp"
class TicoBig : public NPCActor {
public:
    TicoBig(const char*);
    virtual ~TicoBig();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
};
