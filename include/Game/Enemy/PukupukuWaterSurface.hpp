#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/MtxUtil.hpp"

class AnimScaleController;
class PartsModel;
class WalkerStateBindStarPointer;
class WaterInfo;
class YoshiLockOnTarget;

// SMG2-only water-surface Pukupuku (written from the target asm). Its nerve states
// (2FE8..3004 in the target) are the PukupukuWaterSurface*Nrv* classes in the .cpp.
class PukupukuWaterSurface : public LiveActor {
public:
    PukupukuWaterSurface(const char*);

    virtual ~PukupukuWaterSurface();
    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exePukuMoveRail();
    void exeMoveReverse();
    void exePukuBlownOff();
    void exePukuTrampled();
    void exePukuLanding();
    void exePukupukuDead();
    s32 canBeEaten();
    s32 probeSurfaceBelow();
    void setBlowVelocity(HitSensor*, HitSensor*);
    void updatePukuWaterInfo();
    void updateUpVecFromWater();
    void snapToWaterSurface();
    void calcSurfaceMtx(TPos3f* pMtx, LiveActor* pActor, const TVec3f& up, const TVec3f& front);
    void updateRailMove(f32 speed, f32 turn);
    f32 getWaterScale();

    /* 0x90 */ PartsModel* mPartsModel;
    /* 0x94 */ WaterInfo* mWaterInfo;
    /* 0x98 */ TVec3f mFrontVec;
    /* 0xA4 */ TVec3f mUpVec;
    /* 0xB0 */ f32 mNerveRate;
    /* 0xB4 */ f32 _B4;
    /* 0xB8 */ f32 _B8;
    /* 0xBC */ f32 mNerveBlendRate;
    /* 0xC0 */ s32 mLandingCount;
    /* 0xC4 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0xC8 */ AnimScaleController* mScaleCtrl;
    /* 0xCC */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0xD0 */ const Nerve* mRestoreNerve;
};
