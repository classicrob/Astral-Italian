#pragma once

#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/MtxUtil.hpp"

// SMG2-only boss "BellyDragon" (ベリードラゴン, Koopa Jr. castle boss family). No Petari counterpart.
// Layout from the target asm (BellyDragon.s and the TU01..TU06 splits). Offsets are absolute.

class BellyDragon;
class BellyDragonBody;   // TU02, size 0x100, ctor (BellyDragon*, TMtx34*, bool lv2)
class BellyDragonPod;    // TU05, size 0x80, ctor (BellyDragon*, JMapInfoIter*)
class BellyDragonArm;    // TU05, size 0x24, ctor (BellyDragon*, s32 side)
class ActorCameraInfo;
class CameraTargetMtx;
class LiveActorGroup;
class ModelObj;

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
