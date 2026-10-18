#pragma once

#include "Game/Enemy/KameckBeam.hpp"

class JetTurtle;
class SmallKameckBeamEventListener;

class KameckTurtle : public LiveActor, public KameckBeamProjectile {
public:
    /// @brief Creates a new `KameckTurtle`.
    /// @param pName A pointer to the null-terminated name of the object.
    KameckTurtle(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void initJetTurtle();
    virtual void appearDirection(const TVec3f&, const TVec3f&);
    virtual bool fn_801A74D0();
    virtual void fn_801A7180();
    virtual bool fn_801A78B0() const;
    void exeRun();
    void exeWait();
    bool isEnableAttack() const;
    void appearJetTurtle(bool);

    /* 0x98 */ JetTurtle* mTurtle;
    /* 0x9C */ TQuat4f mRotateQuat;
    /* 0xAC */ TVec3f _A4;
    /* 0xB8 */ TVec3f _B0;
};
