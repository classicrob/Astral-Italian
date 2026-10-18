#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class BenefitItemObj;
class CollisionParts;

class TreasureBoxCracked : public LiveActor {
public:
    TreasureBoxCracked(const char*);

    virtual ~TreasureBoxCracked();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeOpenWait();
    void exeAlwaysOpen();
    void initItem(const JMapInfoIter&);
    void makeOpen();
    bool isHalfOpen() const;

    /* 0x90 */ CollisionParts* mBoxCollider;
    /* 0x94 */ TPos3f mTopMtx;
    /* 0xC4 */ s32 mBoxType;
    /* 0xC8 */ s32 mOpenCondition;
    /* 0xCC */ BenefitItemObj* mItem;
    /* 0xD0 */ s32 mItemType;
    /* 0xD4 */ s32 mWaitForSwitchOn;
};
