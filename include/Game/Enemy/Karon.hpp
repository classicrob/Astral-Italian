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
    void exeTurnOnRail();
    void exePursueOnRail();
    bool isRailPursueAllowed() const;
    bool isKaronYoshiEatEnabled() const;
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

    /* 0x90 */ TerritoryMover* mTerritoryMover;
    /* 0x94 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0x98 */ bool mStopSwAnim;
    /* 0x99 */ bool mIsRailConstrained;
    /* 0x9C */ f32 mSearchAngle;
    /* 0xA0 */ TVec3f mRailDirection;
    /* 0xA8 */ AnimScaleController* mScaleController;
    /* 0xAC */ WalkerStateBindStarPointer* mStateStarPointer;
};
