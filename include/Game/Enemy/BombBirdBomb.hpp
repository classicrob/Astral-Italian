#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class WalkerStateBindStarPointer;

class BombBirdBomb : public LiveActor {
public:
    /// @brief Creates a new `BombBirdBomb`.
    /// @param pName A pointer to the null-terminated name of the object.
    BombBirdBomb(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);

    void hold(const TVec3f&);
    void start(const TVec3f&, const TVec3f&);

    void exeHold();
    void exeWait();
    void exeExplosion();
    void exeBindStarPointer();

    /* 0x90 */ WalkerStateBindStarPointer* mBindStarPtr;
    /* 0x94 */ TQuat4f _94;
    /* 0xA4 */ TVec3f _A4;
    /* 0xB0 */ s32 _B0;
    /* 0xB4 */ s32 _B4;
};
