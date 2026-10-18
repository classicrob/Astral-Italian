#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class BreakModel;
class YoshiLockOnTarget;

// SMG2-only unit (KickStone: kickable rock that breaks into star pieces or coins). LiveActor = 0x90 bytes.
class KickStone : public LiveActor {
public:
    KickStone(const char* pName);

    virtual ~KickStone();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeBreak();

public:
    /* 0x90 */ BreakModel* mBreakModel;
    /* 0x94 */ bool mIsCoin;
    /* 0x98 */ YoshiLockOnTarget* mLockOnTarget;
    /* 0x9C */ f32 _9C;
    /* 0xA0 */ bool _A0;
    /* 0xA1 */ bool mArg1;
    /* 0xA2 */ bool mArg2;
    /* 0xA4 */ TVec3f mUp;
    /* 0xB0 */ TVec3f mFront;
    /* 0xBC */ TVec3f _BC;
    /* 0xC8 */ TPos3f mMtx;
    /* 0xF8 */ TVec3f mOffset;
};
