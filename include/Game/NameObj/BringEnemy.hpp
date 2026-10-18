#pragma once

#include "Game/LiveActor/LiveActor.hpp"

// Enemy that is pulled in towards the player (SMG2-only). Child object lists are built in init().
class BringEnemy : public LiveActor {
public:
    BringEnemy(const char*);
    virtual ~BringEnemy();

    void init(JMapInfoIter&);
    virtual bool receiveMessage(u32, HitSensor*, HitSensor*);

    void exeBring();
    void exeWait();
    f32 calcNearestEnemy(TVec3f*, const LiveActor*);
    LiveActor* getNearestEnemy() const;
    s32 getEnemyType() const;

    /* 0x90 */ LiveActor* mTargetEnemy;
    /* 0x94 */ LiveActor** mChildren;
    /* 0x98 */ LiveActor** mChildGroups;
    /* 0x9C */ s32 mChildNum;
    /* 0xA0 */ s32 mBringLimit;
    /* 0xA4 */ s32 mBroughtNum;
    /* 0xA8 */ s32 mEnemyType;
};
