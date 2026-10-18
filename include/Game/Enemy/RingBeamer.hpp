#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ModelObj;
class RingBeam;
class AnimScaleController;
class WalkerStateBindStarPointer;

class RingBeamer : public LiveActor {
public:
    RingBeamer(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    bool isNerveTypeActive();
    void syncSwitchOffB();
    void setupAttack();
    bool tryAttack();
    void exeWait();
    void exeAttack();
    void exeInter();

    /* 0x90 */ RingBeam** mBeams;
    /* 0x94 */ AnimScaleController* mScaleCtrl;
    /* 0x98 */ WalkerStateBindStarPointer* mBindStarPointer;
    /*
    bool _90;
    // needed to get _98 at correct offset
    u8 _94[4]; // unused?
    TMtx34f _98;
    */
};
