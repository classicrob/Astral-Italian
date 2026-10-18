#pragma once

#include "Game/NPC/NPCActor.hpp"

class TurnJointCtrl;

class PenguinCoach : public NPCActor {
public:
    PenguinCoach(const char*);
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    void resetPose();
    void exeReaction();

    /* 0x164 */ TurnJointCtrl* mTurnJointCtrl;
};
