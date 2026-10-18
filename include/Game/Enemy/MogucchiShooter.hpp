#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util/MtxUtil.hpp"

class AnimScaleController;
class HomingKiller;
class JointController;
class JointControllerInfo;
class ModelObj;
class WalkerStateBindStarPointer;
class MogucchiSpike;
class Nerve;

// SMG2 MogucchiShooter (the "GliderBazooka" enemy; written from the target asm).
// Layout from the ctor/init stores: 0x90 home pos, 0x9C front, 0xA8 up, 0xB4 rotation,
// 0xC0.. counters and sub-objects.
class MogucchiShooter : public LiveActor {
public:
    MogucchiShooter(const char*);

    virtual ~MogucchiShooter();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void endClipped();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);

    bool fn_801CE3A0(TPos3f* pMtx, const JointControllerInfo&);
    void fn_801CE460();
    void fn_801CE470();
    static void fn_801CE480(TVec3f* pPos, TVec3f* pVel);
    static void fn_801CE530(TVec3f* pFront, const TVec3f& rPos);
    void fn_801CE710();
    void fn_801CE7C0();
    void fn_801CE820();
    void fn_801CE8C0();
    void fn_801CE9F0();
    void fn_801CEA90();
    void fn_801CEB10();
    void fn_801CEB60();
    void fn_801CEBE0();
    void fn_801CECD0();
    bool fn_801CED80(const TVec3f& rPos);
    s32 fn_801CEF20();
    s32 fn_801CEFC0();
    s32 fn_801CF070();
    s32 fn_801CF0C0();
    s32 fn_801CF120();
    s32 fn_801CF1A0();
    s32 fn_801CF220();

    /* 0x90 */ TVec3f mHomePos;
    /* 0x9C */ TVec3f mFront;
    /* 0xA8 */ TVec3f mUpVec;
    /* 0xB4 */ TVec3f mRot;
    /* 0xC0 */ s32 mTimer;
    /* 0xC4 */ s32 mCounter;
    /* 0xC8 */ s32 mStep;
    /* 0xCC */ s32 mLimit;
    /* 0xD0 */ s32 mSwitchKind;
    /* 0xD4 */ s32 mFlag;
    /* 0xD8 */ s32 mDelay;
    /* 0xDC */ MogucchiSpike* mSpike;
    /* 0xE0 */ ModelObj* mBreakModel;
    /* 0xE4 */ JointController* mJointCtrl;
    /* 0xE8 */ HomingKiller* mHomingKiller;
    /* 0xEC */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0xF0 */ const Nerve* mRestoreNerve;
};

// Spike that hangs off the shooter's "Bazooka" joint (SMG2 vtable __vt__13MogucchiSpike).
class MogucchiSpike : public PartsModel {
public:
    MogucchiSpike(LiveActor*, const char*, const char*, MtxPtr, int, bool);

    virtual void calcAndSetBaseMtx();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    /* 0x9C */ AnimScaleController* mScaleCtrl;
};
