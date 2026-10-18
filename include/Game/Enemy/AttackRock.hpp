#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class AttackRockFinal;
class ProjmapEffectMtxSetter;

// Base rock (the one that falls, gets bound and can be broken). Offsets are SMG2 (LiveActor = 0x90).
class AttackRock : public LiveActor {
public:
    AttackRock(const char* pName);

    virtual ~AttackRock();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

public:
    /* 0x90 */ TPos3f mMtx90;
    /* 0xC0 */ TPos3f mMtxC0;
    /* 0xF0 */ TQuat4f mQuatF0;
    /* 0x100 */ TQuat4f mQuat100;
    /* 0x110 */ LiveActor* mBreakModel;  // BreakModel (LiveActor vtable: appear 0x2C, makeActorDead 0x38)
    /* 0x114 */ LiveActor* mDemoObj;     // DemoObj (LiveActor)
    /* 0x118 */ ProjmapEffectMtxSetter* mProjmapEffectMtxSetter;
    /* 0x11C */ LiveActor* mStateFall;
    /* 0x120 */ LiveActor* mStateHold;
    /* 0x124 */ s32 mRockNum;
    /* 0x128 */ bool mIsOnPlayer;
};
