#pragma once
#include "Game/NPC/NPCActor.hpp"

class KoopaNpc : public NPCActor {
public:
    KoopaNpc(const char*);
    virtual ~KoopaNpc();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    bool tryWakeUp(u32, HitSensor*, HitSensor*);
    void exeSwoonWait();
    void exeSwoonWaitEnd();
    void exeAngryStart();
    void exeAngry();
    void exeAngryEnd();
};
