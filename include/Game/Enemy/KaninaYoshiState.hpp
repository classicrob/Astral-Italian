#pragma once
#include "Game/LiveActor/ActorStateBase.hpp"
#include "JSystem/JGeometry/TMatrix.hpp"
class Kanina;
class LiveActor;
class HitSensor;

class KaninaYoshiState : public ActorStateBase<Kanina> {
public:
    KaninaYoshiState(Kanina*, bool);
    virtual ~KaninaYoshiState();
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    bool tryStartTongue();
    bool tryKill();
    bool receiveMsgYoshi(u32, HitSensor*, HitSensor*);
    bool isYoshiNerve() const;
    void followYoshiMouth();
    void exeWait();
    void exeEaten();
    void exeShot();
    void exeShotStraight();
    void startShot();
    void calcShotPosition(TVec3f*, f32);
    void exeShotArc();
    void exeFall();
    void endFall();
    void exeTrampled();
    void exeTongue();

    s32 mShotMode;
    LiveActor* mClaw;
    bool mHit;
    TVec3f mLocalVelocity;
    TVec3f mFront;
    bool mStopMovement;
    f32 mArcHeight;
    s32 mArcStep;
    TPos3f mShotMtx;
};
