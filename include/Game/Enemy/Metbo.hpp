#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/ActorMovementUtil.hpp"

class SpinHitController;
class YoshiLockOnTarget;
class AnimScaleController;
class WalkerStateBindStarPointer;

class Metbo : public LiveActor {
public:
    Metbo(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeNonActive();
    void endNonActive();
    void exeWait();
    void exeWalkAround();
    void exeSearch();
    void exeChaseStart();
    void exeChase();
    void endChase();
    void exeGuard();
    void exeTrampleDown();
    void exeBlowDown();
    void exeAttack();
    void exeRest();
    void exeSwoonStart();
    void exeSwoonLand();
    void exeSwoonSpin();
    void exeSwoonWait();
    void exeRecover();
    void moveOrFall(const MR::ActorMoveParam&, const TVec3f*);
    void startSwoon(bool);
    bool tryNonActive();
    bool trySearch();
    bool tryDPDSwoon();
    bool isNrvNormal() const;
    bool fn_801CBD00() const;
    bool isEatable() const;

    inline bool isNrvDown() const;
    inline bool isNrvSwoon() const;

    /* 0x8C */ SpinHitController* mSpinHitController;
    /* 0x90 */ AnimScaleController* mAnimScaleController;
    /* 0x94 */ WalkerStateBindStarPointer* mStarPointerBind;
    /* 0x98 */ WalkerStateBindStarPointer* mStateDPDSwoon;
    /* 0x9C */ TVec3f _98;
    /* 0xA8 */ TVec3f _A4;
    /* 0xB4 */ TVec3f _B0;
    /* 0xC0 */ bool mIsClipped;
    /* 0xC4 */ YoshiLockOnTarget* mYoshiLockOn;
};
