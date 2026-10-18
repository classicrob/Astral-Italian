#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class KeySwitch;
class WalkerStateBindStarPointer;

class MechanicKoopaMini : public LiveActor {
public:
    MechanicKoopaMini(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void updateHitSensor(HitSensor*);
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void onAppearSwitchOn();
    void exeWaterBreak();
    bool fn_801C34F0();
    bool fn_801C3590();
    void exeWait();
    void exeWalkOnRail();
    void exeTurn();
    void exeJumpStart();
    void exeJumpEnd();
    void exeFind();
    void exePursue();
    void exePreFireAttack();
    void exeFireAttack();
    void endFireAttack();
    void exeFireAttackEnd();
    void exePreFireAttackAfterBind();
    void exeAttackHit();
    void exeSpinHit();
    void exeTrample();
    void exeHipDropped();
    void exeBindStarPointer();

    /* 0x8C */ bool mIsAppearJumping;
    /* 0x8D */ bool mIsForbidPause;
    /* 0x8E */ bool _8E;
    /* 0x94 */ KeySwitch* mKeySwitch;
    /* 0x98 */ AnimScaleController* mScaleController;
    /* 0x9C */ WalkerStateBindStarPointer* mBindStarPointer;
};
