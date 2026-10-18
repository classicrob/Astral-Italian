#pragma once

#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/MtxUtil.hpp"

// SMG2-only boss "BellyDragon" (ベリードラゴン, Koopa Jr. castle boss family). No Petari counterpart.
// Layout from the target asm (BellyDragon.s and the TU01..TU06 splits). Offsets are absolute.

class BellyDragon;
class BellyDragonBody;   // TU02, size 0x100, ctor (BellyDragon*, TPos3f*, bool lv2)
class BellyDragonBodyPart;   // TU02, LiveActor, size 0x1C0
class BellyDragonWeakPoint;  // not yet split (ctor fn_80097900), LiveActor, size 0xC8
class BreakModel;
class HitSensor;
class BellyDragonPod;    // TU05, size 0x80, ctor (BellyDragon*, JMapInfoIter*)
class BellyDragonArm;    // TU05, size 0x24, ctor (BellyDragon*, s32 side)
class ActorCameraInfo;
class CameraTargetMtx;
class LiveActorGroup;
class ModelObj;
class JointController;
struct JointControllerInfo;

// TU05: pod (camera + effect groups + decoration model). Plain class, no vtable; mHost is its first word.
class BellyDragonPod {
public:
    BellyDragonPod(BellyDragon*, const JMapInfoIter*);

    /* 0x00 */ BellyDragon* mHost;
    /* 0x04 */ ActorCameraInfo* mCameraInfo;
    /* 0x08 */ void* mFlameCtrl;
    /* 0x0C */ TPos3f mBaseMtx;
    /* 0x3C */ TPos3f mHostMtx;
    /* 0x6C */ ModelObj* mDecoration;
    /* 0x70 */ CameraTargetMtx* mCameraTargetMtx;
    /* 0x74 */ s32 mEffectCount;
    /* 0x78 */ LiveActorGroup* mGroupA;
    /* 0x7C */ LiveActorGroup* mGroupB;
};

// TU05: arm (ActorStateBaseInterface with 3 sub-states). Ctor (host, side); side 1 = A, 0 = B.
class BellyDragonArm : public ActorStateBaseInterface {
public:
    BellyDragonArm(BellyDragon*, s32);

    virtual ~BellyDragonArm();
    virtual void init();
    virtual void appear();

    void exeWait();
    void exeFire();
    void exeShot();
    void exeAttack();

    /* 0x0C */ BellyDragon* mHost;
    /* 0x10 */ u8 mSide;
    /* 0x14 */ ActorStateBaseInterface* mStateA;   // 0x28 bytes, ctor fn_80097520
    /* 0x18 */ ActorStateBaseInterface* mStateB;   // 0x48 bytes, ctor fn_80096060
    /* 0x1C */ ActorStateBaseInterface* mStateC;   // 0x58 bytes, ctor fn_80096960
    /* 0x20 */ s32 mCounter;
};
// TU04: group-B actor (pod child, ctor fn_80094020). LiveActor with the owning host at 0x90 and a base matrix at 0x94.
class BellyDragonPodActor : public LiveActor {
public:
    BellyDragonPodActor(BellyDragon*);

    virtual ~BellyDragonPodActor();

    void exeWait();

    /* 0x90 */ BellyDragon* mHost;
    /* 0x94 */ TPos3f mBaseMtx;
};

class BellyDragonState; // TU06, size 0x1C, ActorStateBaseInterface state (ctor BellyDragon*, const char*, const f32*)

// TU06: ActorStateBaseInterface state. Vtable 0x8 dtor, 0xC init, 0x10 appear, 0x14 kill, 0x18 update, 0x1C control.
class BellyDragonState : public ActorStateBaseInterface {
public:
    BellyDragonState(BellyDragon*, const char*, const f32*);

    virtual ~BellyDragonState();
    virtual void init();
    virtual void appear();

    void setOverlay();
    void clearActive();
    void requestDeleteEffect();
    void exeWait();
    void exePlay();

    /* 0x0C */ BellyDragon* mHost;
    /* 0x10 */ const char* mStateName;
    /* 0x14 */ bool mIsActive;
    /* 0x15 */ bool mIsOverlay;
    /* 0x16 */ bool mIsDeleteEffect;
    /* 0x18 */ const f32* mpTime;
};

// Hill-shaped body part of the BellyDragon (LiveActor). Owner is a matrix pointer (0x90), its translation is copied to the actor.
class BellyDragonBodyHill : public LiveActor {
public:
    BellyDragonBodyHill(const char*, MtxPtr, bool, bool);

    virtual ~BellyDragonBodyHill();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void calcAndSetBaseMtx();

    void exeAppear();
    void exeDisappear();

    /* 0x90 */ MtxPtr mParentMtx;
    /* 0x94 */ bool mIsDiveIn;
    /* 0x95 */ bool mIsCheckAppearEnd;
    /* 0x96 */ bool mIsAutoDisappear;
    /* 0x97 */ bool mIsLongDisappear;
    /* 0x98 */ bool mIsDiveSound;
};

// TU02: one segment of the dragon's body chain (head = index 0, 0x20 segments). Follows the previous segment with springs.
class BellyDragonBodyPart : public LiveActor {
public:
    BellyDragonBodyPart(s32, BellyDragonBodyPart*, bool, f32, f32, f32);

    virtual ~BellyDragonBodyPart();
    virtual void init(const JMapInfoIter&);
    virtual MtxPtr getBaseMtx() const;
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void pullToMtx(MtxPtr, f32);
    void pullToMtxSide(MtxPtr, bool, f32, f32, f32);
    void exeWait();
    void exeFollow();
    void exeBindA();
    void exeBindAWeak();
    void exeExitA();
    void exeBindB();
    void exeExitB();
    bool hasWeakPoint() const NO_INLINE;
    void updateDirection();
    void pullTailToMtx(MtxPtr, f32);
    void startBindA(const TPos3f&);
    void startBindB(const TPos3f&);
    bool tryBindA();
    bool tryBindB();
    bool isHead() const NO_INLINE;
    bool isSecond() const NO_INLINE;
    bool isTailSide() const NO_INLINE;
    bool isBindAWeak() const NO_INLINE;
    bool isBindNerve() const NO_INLINE;
    void stop(bool);
    void resume();
    bool calcJoint(TPos3f*, const JointControllerInfo&);
    void pushFromSensor(HitSensor*, HitSensor*, f32);

    /* 0x90 */ bool mIsLv2;
    /* 0x91 */ bool mIsAttackable;
    /* 0x94 */ s32 mIndex;
    /* 0x98 */ f32 mRadius;
    /* 0x9C */ f32 mRate;
    /* 0xA0 */ BellyDragonBodyPart* mPrev;
    /* 0xA4 */ BellyDragonBodyPart* mNext;
    /* 0xA8 */ TPos3f mMtx;
    /* 0xD8 */ TPos3f mBindMtxA;
    /* 0x108 */ TPos3f mBindMtxB;
    /* 0x138 */ BellyDragonWeakPoint* mWeakPoint;
    /* 0x13C */ bool mIsFrozen;
    /* 0x140 */ const f32* mpTurnSpeed;
    /* 0x144 */ const s32* mpBindStep;
    /* 0x148 */ JointController* mJointCtrl;
    /* 0x14C */ f32 mScale;
    /* 0x150 */ BreakModel* mBreakA;
    /* 0x154 */ BreakModel* mBreakB;
    /* 0x158 */ BellyDragonBodyHill* mHillA;
    /* 0x15C */ BellyDragonBodyHill* mHillB;
    /* 0x160 */ TPos3f mHillMtxA;
    /* 0x190 */ TPos3f mHillMtxB;
};

// PLACEHOLDER(core): weak-point actor of the body, its TU is not split/sourced yet (ctor fn_80097900); resolve when that TU gets a src
class BellyDragonWeakPoint : public LiveActor {
public:
    BellyDragonWeakPoint(HitSensor*, BellyDragonBodyPart*);

    void setNerveChance();    // fn_80097A10
    void setNerveWaitA();     // fn_80097A20
    void setNerveBurn();      // fn_80097A30
    void setNerveWaitB();     // fn_80097A40
    void updateMtx();         // fn_80097BE0

    /* 0x90 */ TPos3f mMtx;
    /* 0xC0 */ HitSensor* mSensor;
    /* 0xC4 */ BellyDragonBodyPart* mPart;
};

// TU02: the whole body chain (size 0x100), owned by BellyDragon.
class BellyDragonBody {
public:
    BellyDragonBody(BellyDragon*, TPos3f*, bool);

    void appear();
    void kill();
    void end();
    void freeze();
    void unfreeze();
    void setAttackable();
    void update();
    void registerJointCallBacks();
    void appearJawHill();
    void startHeadAttackHill();
    void appearHeadHill();
    void breakHeadHill();
    void startBindB();
    void forceDeleteEffects();
    void updateTailSpray();

    /* 0x00 */ bool mIsLv2;
    /* 0x04 */ TPos3f* mBaseMtx;
    /* 0x08 */ BellyDragonBodyPart* mHead;
    /* 0x0C */ s32 mPartNum;
    /* 0x10 */ BellyDragonBodyPart** mParts;
    /* 0x14 */ s32 mWeakPointNum;
    /* 0x18 */ BellyDragonWeakPoint** mWeakPoints;
    /* 0x1C */ BreakModel* mBreakHeadHillAppear;
    /* 0x20 */ TPos3f mMtxA;
    /* 0x50 */ TPos3f mMtxB;
    /* 0x80 */ BreakModel* mBreakA;
    /* 0x84 */ TPos3f mMtxC;
    /* 0xB4 */ BreakModel* mBreakB;
    /* 0xB8 */ LiveActor* mHeadHill;
    /* 0xBC */ BreakModel* mBreakHeadHill;
    /* 0xC0 */ BellyDragonBodyHill* mHillA;
    /* 0xC4 */ BellyDragonBodyHill* mHillB;
    /* 0xC8 */ BreakModel* mBreakJawHill;
    /* 0xCC */ BreakModel* mBreakJawAppear;
    /* 0xD0 */ TPos3f mMtxD;
};

class BellyDragon : public LiveActor {
public:
    BellyDragon(const char*);

    virtual ~BellyDragon();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual MtxPtr getBaseMtx() const;

    void emitFireBodyEffects();
    void updateFrontVec(f32);

    void exeStart();
    void exeFire();
    void exeShot();
    void exeAttack();

    /* 0x90 */ bool mIsLv2;
    /* 0x94 */ TVec3f mFront;
    /* 0xA0 */ TPos3f mBaseMtx;
    /* 0xD0 */ BellyDragonBody* mBody;
    /* 0xD4 */ BellyDragonPod* mPod;
    /* 0xD8 */ ActorJointCtrl* mJointCtrl;
    /* 0xDC */ BellyDragonArm* mArmA;
    /* 0xE0 */ BellyDragonArm* mArmB;
    /* 0xE4 */ LiveActor* mDemoB;
    /* 0xE8 */ LiveActor* mDemoA;
};
