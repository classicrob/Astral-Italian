#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class BreakModel;
class GemCageS : public LiveActor {
public:
    GemCageS(const char*);
    virtual ~GemCageS();
    virtual void init(const JMapInfoIter&);
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    bool appearItem();
    void exeWait();
    void exeBreak();
    BreakModel* mBreakModel;
    s32 mItemType;
    bool mIsRockCage;
};
