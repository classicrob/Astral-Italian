#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ActorPoseTQ;
class AnimScaleController;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;

// SMG2-only unit (DanceFlower, rail-riding flower that dances and reacts to Yoshi). LiveActor = 0x90 bytes.
class DanceFlower : public LiveActor {
public:
    // SMG2-only functions (original names unknown; named by address)
    bool fn_8017A520();
    void fn_8017A660();
    bool fn_8017A720() const;
    s8 fn_8017A7F0() const;
    bool fn_8017A850() const;
    bool fn_8017A920() const;
    bool fn_8017A980() const;
    bool fn_8017AA50() const;
    bool fn_8017AB10() const;
    bool fn_8017ABE0() const;
    bool fn_8017AC90() const;
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
