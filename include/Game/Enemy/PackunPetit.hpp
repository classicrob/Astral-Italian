#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class AnimScaleController;
class ModelObj;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;

class PackunPetit : public LiveActor {
public:
    PackunPetit(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeNonActive();
    void endNonActive();
    void exeWait();
    void exeFind();
    void exeThreat();
    void exeTurn();
    void exeAttackStart();
    void exeAttack();
    void exeAttackBack();
    void exeHitWaitForAttack();
    void exeHit();
    void exeTrampleDown();
    void exePunchDown();
    void exeSwoonStart();
    void exeSwoon();
    void exeSwoonToThreat();
    void exeDPDSwoon();

    void initBlowModel();
    void exeYoshiPull();
    void exeYoshiPullEnd();
    void exePetitEaten();
    void startTrampleDown();
    void startPunchDown();
    bool isTongueGrabbableNerve() const;
    bool isTongueHittableNerve() const;
    void setBlownModelPosture(const TPos3f&, f32);
    void punchDown(u32, HitSensor*, HitSensor*);
    void selectNrvWait();
    bool tryNonActive();
    bool tryTurn();
    bool tryDPDSwoon();

    /* 0x8C */ AnimScaleController* mScaleController;
    /* 0x90 */ WalkerStateBindStarPointer* mStarPointerState;
    /* 0x98 */ TVec3f mFront;
    /* 0xA0 */ ModelObj* mBlownModel;
    /* 0xA8 */ bool mDontTurn;
    /* 0xA9 */ bool mUseGroundMovement;
    /* 0xAA */ u8 mReservedAA[2];
    /* 0xAC */ TPos3f mPullMtx;
    /* 0xDC */ TPos3f mJointMtx;
    /* 0x10C */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0x110 */ TVec3f mLastPlayerPos;
};
