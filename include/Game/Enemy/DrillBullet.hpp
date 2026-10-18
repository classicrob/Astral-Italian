#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class HitSensor;
class PartsModel;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;

// SMG2-only unit (DrillBullet, rail-riding drill that digs through walls). LiveActor = 0x90 bytes in SMG2.
class DrillBullet : public LiveActor {
public:
    DrillBullet(const char* pName);

    virtual ~DrillBullet();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorDead();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeMove();
    void exeDigWallIn();
    void exeDigWallOut();
    void exeStepDown();
    void exeReaction();
    void exeEaten();

    bool tryDigWall();
    bool tryBindStarPointer();
    bool tryEaten();
    bool isTargetable() const;

public:
    /* 0x90 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x94 */ WalkerStateBindStarPointer* mStarPointBind;
    /* 0x98 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0x9C */ PartsModel* mPartsModel;
    /* 0xA0 */ TVec3f mRailClip;
    /* 0xAC */ s32 mIsNoGravity;
};
