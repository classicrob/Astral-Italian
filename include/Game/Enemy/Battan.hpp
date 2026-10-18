#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/CollisionPartsFilter.hpp"

class BreakModel;
class SupportTicoStop;
class Battan;

// Rail-riding Battan with five collision parts and four break parts.
// LiveActor = 0x90 bytes in SMG2; members start at 0x90.

// Collision filter built in Battan::init: rejects the Battan's own collision parts.
class BattanPartsFilter : public CollisionPartsFilterBase {
public:
    BattanPartsFilter(Battan* pHost, bool (Battan::*callback)(const CollisionParts*) const);

    virtual bool isInvalidParts(const CollisionParts*) const;

    /* 0x04 */ Battan* mHost;
    /* 0x08 */ bool (Battan::*mCallback)(const CollisionParts*) const;
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
    void fn_801597E0();
    void exeTurn();
    void fn_80159980();
    void fn_80159A00();
    void fn_80159B20();
    void fn_80159BF0();
    void exeAttackSign();
    void fn_80159DE0();
    void exeWaitFall();
    void fn_80159F60();
    void fn_8015A000();
    void fn_8015A0F0();

    void fn_8015A310();
    void fn_8015A410();
    void fn_8015A500();
    void fn_8015A600();
    void killPartsAll();
    void onCollisionPartsAll();
    void offCollisionPartsAll();
    bool fn_8015A850(const CollisionParts* pParts) const;
    bool tryRailEndTurn();
    bool fn_8015AB40() const;
    bool fn_8015ABD0();
    void fn_8015ACE0(f32 speed);
    bool isRailEndReached() const;
    void fn_8015AEE0();
    bool fn_8015B130(TVec3f* pPos, s32 index);
    bool fn_8015B2D0(TVec3f* pDir, const TVec3f* pRange);

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
