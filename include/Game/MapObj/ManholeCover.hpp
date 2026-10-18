#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class MapObjConnector;

class ManholeCover : public LiveActor {
public:
    ManholeCover(const char* pName);

    virtual ~ManholeCover();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeOpen();
    void exeRattle();

    /* 0x90 */ MapObjConnector* mConnector;
    /* 0x94 */ u8 _94;  // set by a 0xD1 message, read on the next player attack
    /* 0x95 */ u8 _95;  // set by a 0xD2 message, moved to _94 in control()
};
