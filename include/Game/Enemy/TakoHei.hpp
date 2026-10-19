#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class HitSensor;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;

class TakoHei : public LiveActor {
public:
    TakoHei(const char*);

    virtual ~TakoHei();
    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    bool requestPressDown();
    bool requestHipDropDown();
    bool requestFlatDown();
    bool requestSwoon();
    bool requestPunch(u32, HitSensor*, HitSensor*);
    bool requestAttackSuccess(HitSensor*, HitSensor*);

    bool tryActive();
    bool tryNonActive();
    bool tryWalk();
    bool tryWalkEnd();
    bool tryFindTurn();
    bool tryFind();
    bool tryPursue();
    bool tryPursueEnd();
    bool tryAttack();
    bool tryCoolDownEnd();
    bool tryAttackSuccessEnd();
    bool tryPointBind();
    bool tryRecover();
    bool tryRecoverEnd();
    bool tryPressed();

    void exeNonActive();
    void exeWait();
    void exeWalk();
    void exeFindTurn();
    void exeFind();
    void exePursue();
    void exeCoolDown();
    void exeAttackSign();
    void exeAttack();
    void exeAttackSuccess();
    void exeBindStarPointer();
    void exeSwoon();
    void exeRecover();
    void exePressDown();
    void exeHipDropDown();
    void exeFlatDown();
    void exePunchDown();

    void updatePose();
    void updateNormalVelocity();
    void decideNextTargetPos();

    bool checkFront() const;
    bool canAttack() const;
    bool isPushMovable() const;
    bool isEnableKick() const;
    bool isDown() const;
    bool isFallNextMove() const;
    bool isInSightMario() const;

    /* 0x8C */ AnimScaleController* mScaleController;
    /* 0x90 */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0x98 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0x94 */ TQuat4f mPoseQuat;
    /* 0xA4 */ TVec3f _A4;
    /* 0xB0 */ TVec3f mFront;
    /* 0xBC */ TVec3f mWalkTarget;
    /* 0xC8 */ TVec3f _C8;
    /* 0xD4 */ TVec3f mHomePos;
    /* 0xE0 */ s32 mNoPushStep;
    /* 0xE4 */ s32 _E4;
    /* 0xE8 */ s32 mAirStep;
    /* 0xEC */ u8 _EC;
    /* 0xED */ u8 mHasMoveLimitCollision;
};
