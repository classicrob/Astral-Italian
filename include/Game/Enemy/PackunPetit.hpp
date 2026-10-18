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
    void endDPDSwoon();

    void initStarPointer(const char*);
    void initBlowModel();
    void punchDown(HitSensor*, HitSensor*);
    void selectNrvWait();
    bool tryNonActive();
    bool tryTurn();
    bool tryDPDSwoon();

    /* 0x8C */ AnimScaleController* mScaleController;
    /* 0x90 */ WalkerStateBindStarPointer* mStarPointerState;
    /* 0x94 */ TVec3f _94;
    /* 0xA0 */ ModelObj* mBlownModel;
    /* 0xA8 */ bool mDontTurn;
    /* 0xA9 */ bool _A9;
    /* 0xAA */ u8 _AA[0xDC - 0xAA];
    /* 0xDC */ TPos3f mJointMtx;
    /* 0x10C */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0x110 */ TVec3f _110;
};
