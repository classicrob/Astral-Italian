#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "math_types.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

#include "Game/Util/DiceMover.hpp"

// SMG2-only unit (no Petari source). Layout from the ctor/init stores (size 0x10C+, NameObjFactory uses ItemDice).
// Abstract: ItemDice implements the pure virtuals below (vtable slots 0x70..0x84, all zero in __vt__8DiceBase).
class DiceBase : public LiveActor {
public:
    DiceBase(const char*);

    virtual ~DiceBase() NO_INLINE {}
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    // pure in DiceBase (0x70..0x84)
    virtual const char* getModelName() = 0;
    virtual void initItems(const JMapInfoIter&) = 0;  // slot 0x74: DiceBase::init passes rIter (see NOTES)
    virtual void setAnimFrame() = 0;
    virtual void setResultFrame(u32) = 0;
    virtual bool makeResult(u32) = 0;
    virtual bool isGoodResult(u32) const = 0;

    void exeSpin();
    void exeThrow();
    void exeLand();
    bool tryThrow();

    // unnamed in the target (fn_802B4160 etc.); names are placeholders until namefn/lead renames
    void resetToBaseTrans();
    void createMover(HitSensor*);
    void calcPushVec(const TVec3f&, const TVec3f&);
    bool isSettled();
    void receiveMsgPushImpl(HitSensor*, HitSensor*, f32);
    void startHitStop();
    bool isPushableSensor(HitSensor*);

    /* 0x090 */ DiceMover* mMover;
    /* 0x094 */ TVec3f mBasePos;
    /* 0x0A0 */ TVec3f mBaseRot;
    /* 0x0AC */ f32 mThrowScaleA;  // JMap arg1 / 10
    /* 0x0B0 */ f32 mThrowScaleB;  // JMap arg2 / 10
    /* 0x0B4 */ s32 mStopTimer;
    /* 0x0B8 */ s32 mLandTimer;
    /* 0x0BC */ u32 mBounceCount;
    /* 0x0C0 */ u32 mHitCount;
    /* 0x0C4 */ TPos3f mMtxC4;
    /* 0x0F4 */ s32 mUnkF4;
    /* 0x0F8 */ s32 mUnkF8;
    /* 0x0FC */ TVec3f mSpinVel;
    /* 0x108 */ bool mIsLanded;
    /* 0x109 */ bool mIsGood;
};

