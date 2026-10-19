#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class KirairaChain;
class AnimScaleController;
class WalkerStateBindStarPointer;
class MsgSharedGroup;

class Kiraira : public LiveActor {
public:
    Kiraira(const char*);

    virtual void init(const JMapInfoIter& rIter);
    virtual void calcAndSetBaseMtx();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void openEyes();
    void closeEyes();
    void drift();
    void driftOnRail();
    void explode();

    bool isDown() const;

    void exeWait();
    void exeFaceToMario();
    void exeFaceToMarioAndStare();
    void exeExplode();
    void exeBeExploded();
    void exeDead();
    void exeRecoverSign();
    void exeRecover();

private:
    /* 0x90 */ f32 mRailCoordSpeed;
    /* 0x94 */ TVec3f mRailVec;
    /* 0xA0 */ TVec3f mFront;
    /* 0xAC */ f32 mDriftPhase;
    /* 0xB0 */ bool mIsRail;
    /* 0xB1 */ bool mIsForceDetonated;
    /* 0xB2 */ bool mEyesOpen;
    /* 0xB4 */ MsgSharedGroup* mSharedGroup;
    /* 0xB8 */ KirairaChain* mChain;
    /* 0xBC */ AnimScaleController* mScaleController;
    /* 0xC0 */ WalkerStateBindStarPointer* mStateBindStarPointer;
};
