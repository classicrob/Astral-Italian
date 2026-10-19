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

    bool calcShooterPropellerJoint(TPos3f* pMtx, const JointControllerInfo&);
    void invalidateShooterClipping();
    void validateShooterClipping();
    static void getShooterTargetMotion(TVec3f* pPos, TVec3f* pVel);
    static void aimShooterAtPredictedTarget(TVec3f* pFront, const TVec3f& rPos);
    void exeShooterWaitSwitch();
    void exeShooterAppear();
    void exeShooterHide();
    void exeShooterBreak();
    void exeShooterShotWait();
    void exeShooterShotSign();
    void exeShooterShotSignWait();
    void exeShooterShotStart();
    void appearShooterHomingKiller();
    void fireShooterBullet();
    bool tryBreakShooterAtPosition(const TVec3f& rPos);
    s32 isShooterTargetInView();
    s32 tryHideShooterOnDamage();
    s32 tryHideShooterNearPlayer();
    s32 tryWaitShooterSwitchOff();
    s32 tryAppearShooterAfterDelay();
    s32 tryStartShooterShotSign();
    s32 tryStartShooterShot();

    /* 0x90 */ TVec3f mHomePos;
    /* 0x9C */ TVec3f mFront;
    /* 0xA8 */ TVec3f mUpVec;
    /* 0xB4 */ TVec3f mAimDirection;
    /* 0xC0 */ s32 mSwayTimer;
    /* 0xC4 */ s32 mShotTimer;
    /* 0xC8 */ s32 mPropellerStep;
    /* 0xCC */ s32 mShotInterval;
    /* 0xD0 */ s32 mProjectileKind;
    /* 0xD4 */ s32 mUnusedFlag;
    /* 0xD8 */ s32 mHideDelay;
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
    MogucchiSpike(LiveActor*, MtxPtr);

    virtual void calcAndSetBaseMtx();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    /* 0xA0 */ AnimScaleController* mScaleCtrl;
};
