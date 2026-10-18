#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/CollisionPartsFilter.hpp"

class BreakModel;
class SupportTicoStop;
class Battan;

// SMG2-only unit (Battan, rail-riding crab-like enemy with 5 collision parts and 4 break parts).
// LiveActor = 0x90 bytes in SMG2; members start at 0x90.

// Collision filter built in Battan::init: rejects the Battan's own collision parts.
class BattanPartsFilter : public CollisionPartsFilterBase {
public:
    BattanPartsFilter(Battan* pHost);

    virtual bool isInvalidParts(const CollisionParts*) const;

    /* 0x04 */ Battan* mHost;
    /* 0x08 */ s32 _8;
    /* 0x0C */ s32 _C;
    /* 0x10 */ void* _10;
};

class Battan : public LiveActor {
public:
    Battan(const char* pName);

    virtual ~Battan();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void calcAnim();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual void updateHitSensor(HitSensor* pSensor);

    void exeWait();
    void exeWalk();
    void exeTurn();
    void exeTurnEnd();
    void exeHalfTurn();
    void exeFind();
    void exeRun();
    void exeAttackSign();
    void exeAttack();
    void exeWaitFall();
    void exeRiseSign();
    void exeRise();
    void exeDown();

    void setPartsStateStand();
    void setPartsStateAttack();
    void setPartsStateAllAppear();
    void setPartsStateHalf();
    void killPartsAll();
    void onCollisionPartsAll(LiveActor* pActor);
    void offCollisionPartsAll(LiveActor* pActor);
    bool isBattanPartsSensor(const CollisionParts* pParts) const;
    bool tryRailEndTurn();
    bool tryAttackDown(LiveActor* pActor);
    bool moveRailStep(f32 speed);
    bool isRailEndReached() const;
    void updateLegs();
    bool calcLegPos(TVec3f* pPos, s32 index);
    bool calcPlayerHitSide(TVec3f* pDir, const TVec3f* pRange);

public:
    /* 0x90 */ SupportTicoStop* mTicoStop;
    /* 0x94 */ LiveActor** mCollisionParts;  // [5], SMG2 BattanKingCollision objects
    /* 0x98 */ LiveActor** mPressParts;      // [5], only 0/3/4 are non-null
    /* 0x9C */ BreakModel* mBreakModel;
    /* 0xA0 */ LiveActor** mBreakParts;      // [4]
    /* 0xA4 */ TVec3f mFront;
    /* 0xB0 */ TVec3f mFrontPrev;
    /* 0xBC */ f32 mWalkSpeed;
    /* 0xC0 */ f32 mRunSpeed;
    /* 0xC4 */ s32 mRailPassNo;
    /* 0xC8 */ bool mIsTurnBack;
    /* 0xC9 */ bool mIsDropStarPiece;
    /* 0xCA */ bool mIsPlayerFront;
    /* 0xCB */ bool mIsPlayerSide;
};
