#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class BenefitItemOneUp;
class ModelObj;
class BreakModel;

class WoodBox : public LiveActor {
public:
    WoodBox(const char*);

    virtual ~WoodBox();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeHit();
    void doHit(HitSensor*, HitSensor*);
    void forceReappear();
    void registerBreakModelDemoCast();

    /* 0x90 */ u16 mFloorTouchTimer;
    /* 0x92 */ char pad92[4];
    /* 0x96 */ u16 mHitPoint;
    /* 0x98 */ bool mHasPowerStar;
    /* 0x99 */ bool mIsNoRespawn;
    /* 0x9A */ bool mPlaySolveSE;
    /* 0x9B */ bool _9B;
    /* 0x9C */ Mtx mBaseMtx;
    /* 0xCC */ s32 mCoinCount;
    /* 0xD0 */ s32 mStarBitCount;
    /* 0xD4 */ BreakModel* mBreakModel;
    /* 0xD8 */ ModelObj* mStarDemoModel;
    /* 0xDC */ BenefitItemOneUp* mOneUp;
};
