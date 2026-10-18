#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class BreakModel;
class DemoObj;
class SnowBall;
class WhiteSnowTree;

class SnowMound : public LiveActor {
public:
    SnowMound(const char*);

    virtual ~SnowMound();
    virtual void init(const JMapInfoIter&) override;
    virtual void kill() override;
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) override;
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*) override;
    void initCsvParam();
    bool tryStartMelt();
    void launchSnowBalls();
    void afterKill();
    void exeMelt();
    void exeHide();
    void exeHidden();

    /* 0x90 */ BreakModel* mBreakModel;
    /* 0x94 */ DemoObj* mBreakDustDemo;
    /* 0x98 */ SnowBall** mSnowBalls;
    /* 0x9C */ s32 mSnowBallNum;
    /* 0xA0 */ s32 mCsvValue;
    /* 0xA4 */ bool mIsCsvFlag;
    /* 0xA8 */ WhiteSnowTree* mSnowTree;
    /* 0xAC */ s32 mPowerStarId;
    /* 0xB0 */ bool mIsRiddle;
};
