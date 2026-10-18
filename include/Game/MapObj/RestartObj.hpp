#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class JMapIdInfo;
class MapObjConnector;
class YoshiLockOnTarget;

class RestartObj : public LiveActor {
public:
    RestartObj(const char*);

    virtual ~RestartObj();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void calcAndSetBaseMtx();
    virtual void updateHitSensor(HitSensor*);
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeCollect();
    void exeShake();
    void exeAfter();
    void changeTexture(bool);

    /* 0x90 */ JMapIdInfo* mIdInfo;
    /* 0x94 */ MapObjConnector* mConnector;
    /* 0x98 */ YoshiLockOnTarget* mLockOnTarget;
};
