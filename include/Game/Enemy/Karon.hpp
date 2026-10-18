#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class WalkerStateBindStarPointer;
class TerritoryMover;
class YoshiLockOnTarget;

class Karon : public LiveActor {
public:
    Karon(const char*);

    virtual ~Karon();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    bool isFallNextMove(bool);
    void fn_801B1670();
    void fn_801B1A10();
    bool fn_801B27C0() const;
    bool fn_801B2840() const;
    void exeLavaDown();

    void exeFixWait();
    void exeWait();
    void exeWalk();
    void exeTurn();
    void exeWalkOnRail();
    void exeSearch();
    void exePursue();
    void endPursue();
    void exeBroken();
    void exeStepBroken();
    void exeRecover();
    void exeHitReaction();
    void exeDeath();
    void exeSinkDown();
    void exeBindStarPointer() NO_INLINE;
    void endBindStarPointer();

    /* 0x8C */ TerritoryMover* mTerritoryMover;
    /* 0x90 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0x94 */ bool mStopSwAnim;
    /* 0x95 */ bool _95;
    /* 0x98 */ f32 _98;
    /* 0xA0 */ TVec3f mRailDirection;
    /* 0xA8 */ AnimScaleController* mScaleController;
    /* 0xAC */ WalkerStateBindStarPointer* mStateStarPointer;
};
