#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class MapObjConnector;
class YoshiLockOnTarget;

class SpinLeverSwitch : public LiveActor {
public:
    SpinLeverSwitch(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    void exeWait();
    void exeSwitchOn();
    void exeEnd();

    /* 0x90 */ MapObjConnector* mConnector;
    YoshiLockOnTarget* mYoshiTarget;
    bool mForceAnim;
};
