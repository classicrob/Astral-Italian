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

    void fn_801F21B0();
    void fn_801F2300();
    void fn_801F23B0();
    void fn_801F24D0();
    void fn_801F2610();
    void fn_801F2740();
    s32 fn_801F2E10();
    s32 fn_801F2EC0();
    void fn_801F2F80(HitSensor*, HitSensor*);
    void fn_801F2FF0();
    void fn_801F3040();
    void fn_801F30D0();
    void fn_801F3150(TPos3f* pMtx, const TVec3f& up, const TVec3f& front);
    void fn_801F32A0(f32 speed, f32 turn);
    f32 fn_801F33A0();

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
