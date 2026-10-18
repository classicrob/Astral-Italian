#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ActorPoseTQ;
class AnimScaleController;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;

// SMG2-only unit (DanceFlower, rail-riding flower that dances and reacts to Yoshi). LiveActor = 0x90 bytes.
class DanceFlower : public LiveActor {
public:
    DanceFlower(const char* pName);

    virtual ~DanceFlower();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeWait();
    void exeAttackSign();
    void exeAttack();
    void exeAttackEnd();
    void exeSwoonStart();
    void exeSwoon();
    void exeSwoonEnd();
    void exePressDown();
    void exeBlowDown();
    void exeTongue();
    void exeFreeze();

public:
    /* 0x90 */ ActorPoseTQ* mPose;
    /* 0x94 */ TVec3f mRailClip;
    /* 0xA0 */ AnimScaleController* mAnimScaleCtrl;
    /* 0xA4 */ WalkerStateBindStarPointer* mStarPointBind;
    /* 0xA8 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0xAC */ s8 mIsSwoonStep;
};
