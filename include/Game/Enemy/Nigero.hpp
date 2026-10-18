#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TQuat.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include "Game/LiveActor/HitSensor.hpp"

class AnimScaleController;
class WalkerStateBindStarPointer;
class ParabolicPath;
class YoshiLockOnTarget;
class HitSensor;

/// @brief Nigero (a small enemy that walks toward the player, hides and comes back).
/// Layout from the SB4E01 ctor (LiveActor is 0x90 bytes in SMG2).
class Nigero : public LiveActor {
public:
    Nigero(const char*);
    virtual ~Nigero();

    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeAppear();
    void exeWaitNormal();
    void exeWalk();
    void exeWalkMove();
    void exeSearch();
    void exeFind();
    void exeAttackStart();
    void exeThrow();
    void exeAway();
    void exeSwoon();
    void exeDead();
    void exeHide();
    void exeHidden();
    void exeAttackSuccess();

    bool isEatenNerve() const;
    bool isLockOnNerve() const;
    bool isActiveNerve() const;
    bool isWalkNerve() const;
    bool isBindedNerve() const;
    bool recvEyeAttack(HitSensor*, HitSensor*);
    bool recvItemGet(HitSensor*, HitSensor*);
    bool recvBodyNerve(HitSensor*, HitSensor*);
    bool recvAttackSensor(HitSensor*, HitSensor*);
    bool recvPushDefault(HitSensor*, HitSensor*);
    void recvLostTarget();
    void recvStarPieceEnd();
    void updateUpVec();
    void moveToTarget(const TVec3f&, f32, f32, f32, f32, f32);

    /* 0x90 */ TQuat4f mRotate;
    /* 0xA0 */ TQuat4f mRotateTarget;
    /* 0xB0 */ s32 mIsHolding;
    /* 0xB4 */ HitSensor* mTargetSensor;
    /* 0xB8 */ ParabolicPath* mPath;
    /* 0xBC */ AnimScaleController* mAnimScaleCtrl;
    /* 0xC0 */ WalkerStateBindStarPointer* mBindStar;
    /* 0xC4 */ TVec3f mGroundPos;
    /* 0xD0 */ TVec3f mFront;
    /* 0xDC */ TVec3f mResetPos;
    /* 0xE8 */ s32 mUpCounter;
    /* 0xEC */ s32 mAwayTimer;
    /* 0xF0 */ YoshiLockOnTarget* mYoshiLockOn;
};
