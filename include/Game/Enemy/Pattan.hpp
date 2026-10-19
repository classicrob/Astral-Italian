#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class AnimScaleController;
class BreakModel;
class HitSensor;
class WalkerStateBindStarPointer;

// SMG2-only unit (Pattan, jumping flower-like enemy with a break model). LiveActor = 0x90 bytes.
class Pattan : public LiveActor {
public:
    Pattan(const char* pName);

    virtual ~Pattan();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    bool isPattanAliveState();
    bool canPattanPushMove();
    bool canPattanBindStarPointer();
    bool tryPattanPop();
    bool tryPattanPressDown();
    bool tryPattanGuard(HitSensor*, HitSensor*);
    void tryPattanJump();
    void exePattanAppear();
    void exePattanWait();
    void exePattanFind();
    void exePattanChase();
    void exeAttackSign();
    void exeAttack();
    void exePattanDown();
    void exePattanJump();
    void exePattanGuard();
    void exePattanDead();
    void exePattanPush();
    void exePattanPop();
    void exeFreeze();

public:
    /* 0x90 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x94 */ WalkerStateBindStarPointer* mStarPointBind;
    /* 0x98 */ BreakModel* mBreakModel;
    /* 0x9C */ TPos3f mBaseMtx;
    /* 0xCC */ TVec3f mFront;
    /* 0xD8 */ s32 mTimer;
    /* 0xDC */ s32 mDamageTimer;
};

void appearPattanByJump(LiveActor*, const TVec3f*);
