#pragma once

#include "Game/Enemy/KameckBeam.hpp"

class KameckBeamEventListener;

class KameckFireBall : public LiveActor, public KameckBeamProjectile {
public:
    /// @brief Creates a new `KameckFireBall`.
    /// @param pName A pointer to the null-terminated name of the object.
    KameckFireBall(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    virtual void appearDirection(const TVec3f&, const TVec3f&);
    virtual void resetBeamProjectile();
    virtual bool isBeamProjectileDead() const;
    void setEventListener(KameckBeamEventListener*);

    void exeWait();
    void exeSweep();

    /* 0x98 */ TQuat4f mRotateQuat;
    /* 0xA8 */ u8 mReservedA8[4];
};
