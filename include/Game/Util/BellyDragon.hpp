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
