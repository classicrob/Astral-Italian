#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class CocoNutBall : public LiveActor {
public:
    bool isInAttackOrFreezeNerve() const;
    void launchCoconutTowardPlayer(const TVec3f&, f32, f32, f32);
    void launchCoconutWithVelocity(const TVec3f&, const TVec3f&);
    void launchFallingCoconut(const TVec3f&, const TVec3f&);
    CocoNutBall(const char* pName = "ヤシの実弾");

    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void kill();
    virtual void calcAndSetBaseMtx();
    virtual void control();
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void initSensors();
    void appearAndThrow(const TVec3f&, f32);
    void hitBackToPlayer();
    void demoBreak(const TVec3f&);
    HitSensor* isBindedAny() const;
    bool isValidReceivePunch() const;
    bool isNerveTrowToOrFreeze() const;  // inline used several places
    bool isSensorBody(HitSensor*) const;
    void calcHitBackVelocitAndGravity();
    bool isHitBackRight() const;
    bool isHitBackFront() const;
    void calcHitBackDstPos(TVec3f*, bool, bool);
    bool tryToKill(bool);
    void setVelocityToPlayer(f32, f32, f32);
    void freeze();
    bool isFreezable();
    void processApproachToPlayer();
    void exeThrow();
    void exeHitBackToHost();
    void exeHitBackToPlayer();
    void exeRebound();
    void exeFreezeStart();
    void exeHitBackToOtaRockTank();
    bool canHitBackFromSensor(HitSensor*);
    void calcTankHitBackVelocity();
    void calcPlayerHitBackVelocity(HitSensor*);
    void exeFreeze();
    void exeFreezeRelease();

    /* 0x8C */ LiveActor* mHost;
    /* 0x90 */ TVec3f mAccel;
    /* 0x9C */ s32 mHitBackCount;
    /* 0xA0 */ s32 mFreezeStep;
    /* 0xA4 */ TVec3f mFreezePos;
    /* 0xB0 */ TVec3f mFreezeVelocity;
    /* 0xBC */ bool mHitBackRight;
    /* 0xBD */ bool mCurveRight;
    /* 0xBE */ bool mIsBoundedByHost;
    /* 0xC0 */ f32 mMaxHeightOffset;
    /* 0xC4 */ f32 mMinHeightOffset;
    /* 0xC8 */ TVec3f mUp;
    /* 0xD4 */ f32 mKillDistance;
    /* 0xD8 */ bool mIsRallyBall;
    /* 0xDD */ bool mIsFalling;
    /* 0xDE */ bool mIsTankBall;
    /* 0xE0 */ f32 mFlightSteps;
    /* 0xE4 */ s32 mHitBackLockStep;
};
