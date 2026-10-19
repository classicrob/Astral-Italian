#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class CollisionParts;
class MapObjConnector;
class SpringValue;

class HipDropSwitch : public LiveActor {
public:
    HipDropSwitch(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void initAfterPlacement();
    virtual void calcAnim();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    bool isPlayerOnTop();
    bool trySwitchDown();
    bool tryOn();

    void exeOff();
    void exeSwitchDown();
    void exeOn();

    /* 0x090 */ u32 _90;  // useless
    /* 0x094 */ SpringValue* mSpringValue;
    /* 0x098 */ MapObjConnector* mConnector;
    /* 0x09C */ CollisionParts* mSpringModel;
    /* 0x0A0 */ TPos3f _A0;
    /* 0x0D0 */ bool mIsHipDropped;
    /* 0x0D1 */ bool mWasTouched;
    /* 0x0D2 */ bool mIsTouched;
};
