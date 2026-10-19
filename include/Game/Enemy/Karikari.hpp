#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class FixedPosition;
class AnimScaleController;
class WalkerStateBindStarPointer;

class Karikari : public LiveActor {
public:
    Karikari(const char*);

    virtual ~Karikari();
    virtual void init(const JMapInfoIter& rIter);
    virtual void kill();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void applyEnvironmentInfluenceToVelocity();
    void killedInFrozenState();

    void exeFall();
    void exeLand();
    void exeNoCalcWait();
    void exeWait();
    void exePrePursue();
    void exePursue();
    void exeWatchFor();
    void exePreCling();
    void exeCling();
    void exeRelease();
    void exeSpinAttacked();
    void exeDPDAttacked();
    void exeFrozen();
    void exeFrozenRecover();
    void exeBlowOut();
    void exeInTornado();
    void exePress();

    bool tryBlowOut(const TVec3f&, bool);
    bool tryElectricKill();
    bool tryHipDropRelease();
    bool tryStarPointerBind();


    void tryTurnToDirection(const TVec3f& rDir, f32 turnRatio);

    bool inClingNerve() const;

    /* 0x8C */ TVec3f mFront;
    /* 0x9C */ TQuat4f mPoseQuat;
    /* 0xAC */ FixedPosition* mClingPosition;
    /* 0xB0 */ AnimScaleController* mAnimScale;
    /* 0xB4 */ WalkerStateBindStarPointer* mStarPointer;
    /* 0xB8 */ bool mIsAttackEnabled;
    /* 0xB9 */ bool mIsPushable;
    /* 0xBC */ s32 mFrozenTime;
    /* 0xC0 */ u32 mUnusedC0;
    /* 0xC4 */ bool mIsReadyToLandTornado;
    /* 0xC5 */ bool mUnusedC5;
};
