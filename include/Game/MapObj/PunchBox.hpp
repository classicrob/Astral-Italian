#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class PunchBox : public LiveActor {
public:
    PunchBox(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual void makeActorAppeared();
    virtual void makeActorDead();

    void doHit(HitSensor*, HitSensor*);

    void exeWait();
    void exeHit();

    /* 0x90 */ u16 _8C;  // SMG2 layout (LiveActor 0x90): fields are not a plain +4 shift of Petari
    /* 0x92 */ u16 _90;
    /* 0x94 */ u16 _92;
    /* 0x96 */ u16 _94;
    /* 0x98 */ bool _96;
    /* 0x99 */ bool mHasShadow;  // Obj_arg1
    /* 0x9C */ s32 mCoinCount;   // Obj_arg0
};
