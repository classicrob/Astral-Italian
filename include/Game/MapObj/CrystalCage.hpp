#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class DummyDisplayModel;
class ModelObj;
class RumbleCalculatorCosMultLinear;

class CrystalCage : public LiveActor {
public:
    CrystalCage(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);

    void forceBreak();
    void initMapToolInfo(const JMapInfoIter&);
    void initModel();
    void tryOnSwitchDead();
    void appear();
    bool isDisplayModelId(s32) const;
    bool hasPowerStarId();
    void exeFall();
    void exeWait();
    void exeBreak();
    void exeBreakAfter();

    /* 0x90 */ s32 mCrystalCageType;
    /* 0x94 */ ModelObj* mBreakObj;
    /* 0x98 */ TPos3f mBaseMtx;
    /* 0xC8 */ s32 mRemainingHits;
    /* 0xCC */ s32 mHitCooldown;
    /* 0xD0 */ RumbleCalculatorCosMultLinear* mRumbleCalc;
    /* 0xD4 */ TVec3f mHitDir;
    /* 0xE0 */ TVec3f mBasePos;
    /* 0xEC */ TVec3f mBodySensorPos;
    /* 0xF8 */ DummyDisplayModel* mDisplayModel;
    /* 0xFC */ TVec3f mGroundPos;
    /* 0x108 */ bool mIsKillIfClipped;
    /* 0x109 */ bool mIsInPlaneModeCube;
    /* 0x10A */ bool mIsFallType;
    /* 0x10B */ u8 _107;
    /* 0x10C */ s32 mPowerStarId;
    /* 0x110 */ bool mIsBreakObjVisible;
    /* 0x111 */ u8 _111;
    /* 0x112 */ u8 _112;
    /* 0x113 */ u8 _113;
    /* 0x114 */ s32 mPowerStarState;
    /* 0x118 */ bool mHasBinding;
    /* 0x11C */ TVec3f mBinderOffset;
};
