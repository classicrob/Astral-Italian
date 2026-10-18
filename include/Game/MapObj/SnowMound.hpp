#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class SnowMound : public LiveActor {
public:
    SnowMound(const char*);

    virtual ~SnowMound();
    virtual void init(const JMapInfoIter&) override;
    virtual void kill() override;
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) override;
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*) override;
    void afterKill();
    void exeMelt();
    void exeHide();
    void exeHidden();
    void startMelt();

    /* 0x90 */ LiveActor* mBreakModel;
    /* 0x94 */ LiveActor* mBreakDustDemo;
    /* 0x98 */ LiveActor** mSnowBalls;
    /* 0x9C */ s32 mSnowBallNum;
    /* 0xA0 */ s32 mCsvValue;
    /* 0xA4 */ bool mIsCsvFlag;
    /* 0xA8 */ LiveActor* mSnowTree;
    /* 0xAC */ s32 mPowerStarId;
    /* 0xB0 */ bool mIsMelt;
};
