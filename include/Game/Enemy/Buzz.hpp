#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;

/// @brief Buzz enemy (SMG2 only, no SMG1 source). Walks a rail, can be frozen by a star pointer bind, and can be eaten by Yoshi.
class Buzz : public LiveActor {
public:
    Buzz(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeWait();
    void exeEaten();
    void exeFreeze();
    void exeStarPieceHit();
    void exeAttackSuccess();
    void exeBlowDown();
    void exeStepDown();

    bool isBindableNerve() const;
    bool isWaitNerve() const;
    bool isHittableBuzzNerve() const;
    bool isStarPieceTargetNerve() const;
    bool isActiveBuzzNerve() const;

    /* 0x90 */ TVec3f mRailPos;
    /* 0x9C */ TVec3f mFront;
    /* 0xA8 */ f32 mSpeed;
    /* 0xAC */ AnimScaleController* mAnimScaleController;
    /* 0xB0 */ WalkerStateBindStarPointer* mBindState;
    /* 0xB4 */ YoshiLockOnTarget* mYoshiLockOnTarget;
};
