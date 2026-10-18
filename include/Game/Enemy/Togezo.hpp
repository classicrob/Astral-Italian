#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include <JSystem/JGeometry/TVec.hpp>

// Direction helper for rail-following Togezo (called from outside this unit).
void fn_80214610(TVec3f* pDst, const LiveActor* pActor);

class AnimScaleController;
class ModelObj;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;

namespace MR {
    struct ActorMoveParam;
};

// SMG2-only unit. Offsets are SMG2 (LiveActor = 0x90). Written from the target asm.
class Togezo : public LiveActor {
public:
    Togezo(const char* pName);

    virtual ~Togezo();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual void initActorInfo(const JMapInfoIter&);  // fn_80212870

    void appearBound(const TVec3f&, const TVec3f&);  // fn_80211430
    void appearHold(MtxPtr);                         // fn_802114D0
    void releaseHold(const TVec3f&);                 // fn_80211590
    void setTimer(s32);                              // fn_80211650
    void updateTimer();                              // fn_80211660
    bool isYoshiState() const;                       // fn_80211780
    bool isShot() const;                             // fn_80211810

    void exeHold();                // fn_80211820
    void exeBound();               // fn_80211870
    void exeAppearDirect();        // fn_802119A0
    void exeAppear();              // fn_80211A90
    void exeWalkOnRail();          // fn_80211B40
    void exeTurnBack();            // fn_80211BC0
    void exeWait();                // fn_80211CE0
    void exeWalk();                // fn_80211DD0
    void exeTurn();                // fn_80211F10
    void exeSearch();              // fn_80211FB0
    void exeRun();                 // fn_80212050
    void exeAttack();              // fn_80212110
    void exeAttackEnd();           // fn_802121B0
    void exeDown();                // fn_80212250
    void exeDeath();               // fn_80212310
    void exeSpinReaction();        // fn_80212370
    void exeBindStarPointer();     // fn_80212410
    void exeShellStart();          // fn_80212480
    void exeShellStartLand();      // fn_80212520
    void exeShellWait();           // fn_802125E0
    void exeShot();                // fn_80212680
    void exeTongue();              // inlined in nerve
    void exeTongueHold();          // inlined in nerve
    void exeEaten();               // inlined in nerve
    void exeCancelEaten();         // inlined in nerve

    bool receiveYoshiMsg(u32, HitSensor*, HitSensor*);                      // fn_802133A0
    void moveAndTurn(const MR::ActorMoveParam&, const TVec3f&);             // fn_80213540
    void calcYoshiDirection();                                              // fn_80213600
    void startRoll();                                                       // fn_802136C0
    void calcThrowVelocity();                                  // fn_80213720
    void calcThrowDirection();                                 // fn_80213780
    bool tryTurnBackOrRail();                                               // fn_80213880
    bool tryAttack();                                                       // fn_80213950
    bool tryStartTongue();                                                  // fn_80213A90
    bool isWalking() const;                                                 // fn_80213BD0
    bool isAttackable() const;                                              // fn_80213CB0
    bool isShotEnd() const;                                                 // fn_80213D80

    /* 0x90 */ ModelObj* mPipo;
    /* 0x94 */ TVec3f mFrontVec;
    /* 0xA0 */ TVec3f mHomePos;
    /* 0xAC */ TVec3f mWalkDir;
    /* 0xB8 */ s32 mGroundCounter;
    /* 0xBC */ s32 mTimer;
    /* 0xC0 */ bool mHasAttacked;
    /* 0xC4 */ AnimScaleController* mScaleController;
    /* 0xC8 */ WalkerStateBindStarPointer* mStateBindStarPointer;
    /* 0xCC */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0xD0 */ MtxPtr mHoldMtx;
    /* 0xD4 */ bool mIsYoshi;
};

class TogezoMgr : public DeriveActorGroup< Togezo > {
public:
    TogezoMgr(int, const TVec3f&);  // fn_80214350
    virtual ~TogezoMgr();

    void updateAll();            // fn_802144C0
    s32 getActiveNum() const;    // fn_80214530
};
