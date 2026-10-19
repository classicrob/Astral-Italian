#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class WalkerStateBindStarPointer;
class ScatterFireBallLava;

class ScatterFireBall : public LiveActor {
public:
    ScatterFireBall(const char* = "拡散ファイアーボール");
    virtual ~ScatterFireBall();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    HitSensor* getContactSensor() const;
    bool tryExplodeOnContact();
    void setVelocityTowardPlayer(f32);
    void scatterVelocity();
    void exeScatterThrow();
    void exeScatterReflect();
    void exeFreeze();

    void shootWithSpreadAngle(const TVec3f&, f32, f32);
    void shootWithVelocity(const TVec3f&, const TVec3f&);
    void shootFalling(const TVec3f&, const TVec3f&);
    bool isFullyDead();
    bool tryKillLava();

    /* 0x90 */ LiveActor* mHost;
    /* 0x94 */ AnimScaleController* mScaleCtrl;
    /* 0x98 */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0x9C */ TVec3f mUpVec;
    /* 0xA8 */ TVec3f mStoredVelocity;
    /* 0xB4 */ f32 _B4;
    /* 0xB8 */ ScatterFireBallLava* mLava;
    /* 0xBC */ s32 mNoHitTimer;
    /* 0xC0 */ bool mIsFalling;
};
