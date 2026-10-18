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

// SMG2-only unit (BattanKing boss: 5 collision parts + 5 press parts + a break model, Pattan holder).
// LiveActor = 0x90 bytes in SMG2; members start at 0x90. Vtable is the LiveActor table (no new virtuals).

class BattanKing : public LiveActor {
public:
    BattanKing(const char* pName);

    virtual ~BattanKing();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void calcAnim();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    bool isInvalidTriangle(const Triangle*);
    bool isValidCollisionState(s32) const;
    bool isNearDropPoint(const TVec3f& rPoint, f32 radius, f32 height);

    /* 0x90 */ BattanKingCollision** mCollisions;
    /* 0x94 */ BattanKingCollisionPress** mCollisionPresses;
    /* 0x98 */ BreakModel* mBreakModel;
    /* 0x9C */ TMtx34f mMtxBreak;
    /* 0xCC */ VolumeDrawer** mVolumeDrawers;
    /* 0xD0 */ f32 mUnkD0;
    /* 0xD4 */ f32 mUnkD4;
    /* 0xD8 */ f32 mUnkD8;
    /* 0xDC */ TMtx34f* mVolumeMtxs;
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
    /* 0x1CC */ s8 mFlagA;
    /* 0x1CD */ s8 mFlagB;
    /* 0x1CE */ s8 mFlagC;
};

// Collision part of BattanKing (body/arms/legs). 0x98 bytes.
class BattanKingCollision : public LiveActor {
public:
    BattanKingCollision(BattanKing* pHost, const char* pName, const char* pPartsName, HitSensor* pSensor,
                        MtxPtr pBaseMtx, s32 connectType);

    virtual ~BattanKingCollision();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void movement();
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual MtxPtr getBaseMtx() const;

    /* 0x90 */ BattanKing* mHost;
    /* 0x94 */ MtxPtr mBaseMtx;
};

// Press-version collision part of BattanKing. 0x98 bytes.
class BattanKingCollisionPress : public LiveActor {
public:
    BattanKingCollisionPress(BattanKing* pHost, const char* pName, const char* pPartsName, HitSensor* pSensor,
                             MtxPtr pBaseMtx, s32 connectType);

    virtual ~BattanKingCollisionPress();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void movement();
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual MtxPtr getBaseMtx() const;

    /* 0x90 */ BattanKing* mHost;
    /* 0x94 */ MtxPtr mBaseMtx;
};

// Runtime-initialized .bss constants of the BattanKing TU (set by its __sinit, which is not split yet).
// Provisional names: lbl_80728914 / lbl_80728920 / lbl_8072892C / lbl_80728938 / lbl_80728944 / lbl_8072895C /
// lbl_80728968 in the target.
namespace BattanKingVec {
    extern TVec3f sOffset914;
    extern TVec3f sOffset920;
    extern TVec3f sOffset92C;
    extern TVec3f sDropPoint938;
    extern TVec3f sOrigin944;
    extern TVec3f sRange95C;
    extern TMtx34f sMtx968[7];
}

// Nerves of BattanKing. Executes are defined in the BattanKing translation unit that owns the sInstance objects.
#define BATTAN_KING_NERVE_DECL(name)                                                                                                 \
    class name : public Nerve {                                                                                                      \
    public:                                                                                                                          \
        virtual void execute(Spine* pSpine) const;                                                                                   \
        static name sInstance;                                                                                                       \
    }

namespace NrvBattanKing {
    BATTAN_KING_NERVE_DECL(NrvE68);
    BATTAN_KING_NERVE_DECL(NrvE6C);
    BATTAN_KING_NERVE_DECL(NrvE70);
    BATTAN_KING_NERVE_DECL(NrvE74);
    BATTAN_KING_NERVE_DECL(NrvE7C);
    BATTAN_KING_NERVE_DECL(NrvE94);
    BATTAN_KING_NERVE_DECL(NrvE98);
    BATTAN_KING_NERVE_DECL(NrvE9C);
    BATTAN_KING_NERVE_DECL(NrvEA0);
    BATTAN_KING_NERVE_DECL(NrvEA4);
    BATTAN_KING_NERVE_DECL(NrvEA8);
    BATTAN_KING_NERVE_DECL(NrvEAC);
};

// Talk / demo state of a BattanKing (ActorStateBase<BattanKing>, 0x1C bytes).
class BattanKingTalkState : public ActorStateBase<BattanKing> {
public:
    BattanKingTalkState(BattanKing* pHost, const JMapInfoIter& rIter, const char* pName, const char* pDemoName,
                        const char* pAnimName);

    /* 0x10 */ TalkMessageCtrl* mTalkCtrl;
    /* 0x14 */ const JMapInfoIter* mIter;
    /* 0x18 */ s8 mFlag18;
};

// Group of Pattan (small jump-flower) enemies spawned by BattanKing. Vtable __vt__12PattanHolder.
class PattanHolder : public LiveActorGroup {
public:
    inline PattanHolder(const char* pName, s32 capacity) : LiveActorGroup(pName, capacity) {
    }
};
