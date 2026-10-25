#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/MtxUtil.hpp"

class BattanKing;
class BattanKingCollision;
class BattanKingCollisionPress;
class BattanKingTalkState;
class BreakModel;
class ActorCameraInfo;
class VolumeDrawer;
class PattanHolder;
class TalkMessageCtrl;
class Triangle;
class CollisionParts;

// SMG2-only unit (BattanKing boss: 5 collision parts + 5 press parts + a break model, Pattan holder).
// LiveActor = 0x90 bytes in SMG2; members start at 0x90. Vtable is the LiveActor table (no new virtuals).

class BattanKing : public LiveActor {
public:
    BattanKing(const char* pName);

    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void calcAnim();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void moveAndTurnToPlayer(const f32* pParam);
    void exeWaitStand();
    void exeTurn();
    void exeFindOrSummon();
    void exeRun();
    void exeWaitSummon();
    void exeQuake();
    void exeAttackJump();
    void exeAttackSign();
    void exeAttack();
    void exeWaitFall();
    void exeRiseSign();
    void exeRise();
    void exeDamage();
    void exeWaitSwitch();
    void exeOpeningDemo();
    void exeDead();
    void exeDownDemo();
    void resetVolumes();
    bool isAttackPhaseStart() const NO_INLINE;
    bool tryAdvanceAttack();
    void spawnPattans();
    void updateJointVisibility();
    void updateShockPos();
    void showCollisionPartsNormal();
    void showCollisionPartsSwapped();
    void updateCollisionPartsFootState();
    void hideCollisionPartsAll();
    bool isAnyCollisionOnActor();
    void updateAttackAnim(s32);
    bool calcPlayerPush(TVec3f*);
    void restoreDemoPose();
    void updateDemoPlayerBaseMtx();
    bool isInvalidCollisionParts(const CollisionParts*);
    bool isValidCollisionState(s32) const;
    bool isNearDropPoint(const TVec3f& rPoint, f32 radius, f32 height);

    /* 0x90 */ BattanKingCollision** mCollisions;
    /* 0x94 */ BattanKingCollisionPress** mCollisionPresses;
    /* 0x98 */ BreakModel* mBreakModel;
    /* 0x9C */ TMtx34f mMtxBreak;
    /* 0xCC */ VolumeDrawer** mVolumeDrawers;
    /* 0xD0 */ TVec3f mShake;
    /* 0xDC */ TPos3f* mVolumeMtxs;
    /* 0xE0 */ VolumeDrawer* mEarthquakeDrawer;
    /* 0xE4 */ TMtx34f mEarthquakeMtx;
    /* 0x114 */ TMtx34f mMtxFx;
    /* 0x144 */ TPos3f mMtxFollow;
    /* 0x174 */ ActorCameraInfo* mCameraInfo;
    /* 0x178 */ BattanKingTalkState* mTalkStateA;
    /* 0x17C */ BattanKingTalkState* mTalkStateB;
    /* 0x180 */ TVec3f mUnk180;
    /* 0x18C */ TVec3f mUnk18C;
    /* 0x198 */ PattanHolder* mPattanHolder;
    /* 0x19C */ TVec3f mFront;
    /* 0x1A8 */ s32 mPhase;
    /* 0x1AC */ s32 mAttackType;
    /* 0x1B0 */ TVec3f mUnk1B0;
    /* 0x1BC */ TVec3f mUnk1BC;
    /* 0x1C8 */ s32 mArg7;
    /* 0x1CC */ u8 mFlagA;
    /* 0x1CD */ u8 mFlagB;
    /* 0x1CE */ u8 mFlagC;
};

// Collision part of BattanKing or Battan (body/arms/legs). 0x98 bytes; the host is any LiveActor.
class BattanKingCollision : public LiveActor {
public:
    BattanKingCollision(LiveActor* pHost, const char* pName, const char* pPartsName, HitSensor* pSensor,
                        MtxPtr pBaseMtx, s32 connectType);

    virtual ~BattanKingCollision();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void movement();
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual MtxPtr getBaseMtx() const;

    /* 0x90 */ LiveActor* mHost;
    /* 0x94 */ MtxPtr mBaseMtx;
};

// Press-version collision part of BattanKing or Battan. 0x98 bytes; the host is any LiveActor.
class BattanKingCollisionPress : public LiveActor {
public:
    BattanKingCollisionPress(LiveActor* pHost, const char* pName, const char* pPartsName, HitSensor* pSensor,
                             MtxPtr pBaseMtx, s32 connectType);

    virtual ~BattanKingCollisionPress();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void movement();
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual MtxPtr getBaseMtx() const;

    /* 0x90 */ LiveActor* mHost;
    /* 0x94 */ MtxPtr mBaseMtx;
};

// Nerve declaration used by the BattanKing talk state (BattanKingCollisionPress.hpp).
#define BATTAN_KING_NERVE_DECL(name)                                                                                                 \
    class name : public Nerve {                                                                                                      \
    public:                                                                                                                          \
        virtual void execute(Spine* pSpine) const;                                                                                   \
        static name sInstance;                                                                                                       \
    }

// One volume drawer of BattanKing: scale and offset applied to its base matrix (0x18 bytes).
struct BattanKingVolumeParam {
    /* 0x00 */ TVec3f mScale;
    /* 0x0C */ TVec3f mOffset;
};



// Talk / demo state of a BattanKing (ActorStateBase<BattanKing>, 0x1C bytes).
class BattanKingTalkState : public ActorStateBase<BattanKing> {
public:
    BattanKingTalkState(BattanKing* pHost, const JMapInfoIter& rIter, const char* pName, const char* pDemoName,
                        const char* pAnimName);

    virtual ~BattanKingTalkState();
    virtual void appear();
    void exeTryStart();
    void exeTalk();

    /* 0x10 */ TalkMessageCtrl* mTalkCtrl;
    /* 0x14 */ union { const JMapInfoIter* mIter; const char* mDemoName; };
    /* 0x18 */ u8 mFlag18;
};

namespace BattanKingParam {
    extern const f32 sRun[4];
    extern const f32 sDash[4];
    extern const f32 sTurn[4];
    extern const f32 sReturn[4];
    extern const volatile f32 sAttackTurnRate;
    extern const volatile s32 sInitialPattanCount;
    extern const volatile s32 sPushStartStep;
    extern const volatile s32 sAttackSignShakeFrame;
    extern const volatile s32 sVolumeDrawPasses;
    extern const volatile s32 sEarthquakeDrawPasses;
    extern const GXColor sVolumeColor;
    extern const GXColor sEarthquakeColor;
}
