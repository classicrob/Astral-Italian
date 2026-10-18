#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class BreakModel;

class CrystalCageOpa : public LiveActor {
public:
    CrystalCageOpa(const char*);

    virtual ~CrystalCageOpa();
    virtual void init(const JMapInfoIter&);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeBreak();
    void exeBreakAfter();
    void appearItem();

    /* 0x90 */ BreakModel* mBreakModel;
    /* 0x94 */ s32 mWaitTime;
    /* 0x98 */ bool mIsStartAnim;
    /* 0x99 */ bool mIsHideModel;
    /* 0x9C */ const char* mObjName;
    /* 0xA0 */ s32 mItemType;
    /* 0xA4 */ s32 mStarPieceNum;
};
