#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class WalkerStateBindStarPointer;

class JellyfishElectric : public LiveActor {
public:
    JellyfishElectric(const char*);

    virtual ~JellyfishElectric();
    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeWaitWithLeftTurn();
    void exeWaitWithRightTurn();
    void exeDamage();
    void exeDeath();
    void exeAttack();
    void exeRailGoal();
    void exeDPDSwoon();
    void endDPDSwoon();
    void waitTurn();
    void knockOut();
    bool tryToAttackElectric(HitSensor*, HitSensor*);
    bool tryDPDSwoon();
    bool selectNerveAfterWait();

    /* 0x8C */ AnimScaleController* mController;
    /* 0x90 */ WalkerStateBindStarPointer* mBindStarPtr;
    /* 0x94 */ s32 mWaveStep;
    /* 0x98 */ TVec3f mFront;
    /* 0xA4 */ bool mIsDanger;
    /* 0xA5 */ bool mIsConnectedRail;
    /* 0xA8 */ f32 mRailSpeed;
    /* 0xAC */ s32 mGoalWaitStep;
    /* 0xB0 */ bool mIsLoopRail;
    /* 0xB4 */ TVec3f mRailClipPos;
};
