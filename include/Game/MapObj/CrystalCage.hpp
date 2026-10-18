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
    bool isDisplayModelId(s32);
    bool hasPowerStarId();
    void exeFall();
    void exeWait();
    void exeBreak();
    void exeBreakAfter();

    /* 0x90 */ s32 mCrystalCageType;
    /* 0x94 */ ModelObj* mBreakObj;
    /* 0x98 */ TPos3f _94;
    /* 0xC8 */ s32 _C4;
    /* 0xCC */ s32 _C8;
    /* 0xD0 */ RumbleCalculatorCosMultLinear* mRumbleCalc;
    /* 0xD4 */ TVec3f _D0;
    /* 0xE0 */ TVec3f _DC;
    /* 0xEC */ TVec3f _E8;
    /* 0xF8 */ DummyDisplayModel* mDisplayModel;
    /* 0xFC */ TVec3f _F8;
    /* 0x108 */ bool _104;
    /* 0x109 */ bool mIsInPlaneModeCube;
    /* 0x10A */ bool mIsFallType;
    /* 0x10B */ u8 _107;
    /* 0x10C */ s32 _108;
    /* 0x110 */ bool mIsBreakObjVisible;
    /* 0x111 */ bool mPlayRiddleSFX;
    /* 0x112 */ bool mHasBinding;
    /* 0x113 */ u8 _10F;
    /* 0x114 */ s32 mPowerStarState;
    /* 0x118 */ bool mHasBinder;
    /* 0x11C */ TVec3f _110;
};
