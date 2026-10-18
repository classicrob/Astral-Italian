#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "math_types.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class DiceMover;

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

// Inline-visible fields of the 0xF0-byte helper at DiceBase+0x90 (created by fn_802B41B0).
// Only the members DiceBase.cpp touches are named.
class DiceMover {
public:
    DiceMover(s32, s32);
    void addCollisionPoint(const TVec3f*);
    void setMtx(MtxPtr);
    void collide(const TVec3f*, const TVec3f*);
    void update();
    void calcMtx(TPos3f*);
    void calcRadius();
    void setGravity(const TVec3f*);
    void setPointArray(void*, s32);

    /* 0x000 */ u8 _00[0x7C];
    /* 0x07C */ f32 mSpeedLimit;
    /* 0x080 */ u8 _80[0x0C];
    /* 0x08C */ f32 m8C;
    /* 0x090 */ u8 _90[0x24];
    /* 0x0B4 */ f32 mB4;
    /* 0x0B8 */ u8 _B8[0x1C];
    /* 0x0D4 */ u8 mD4;
    /* 0x0D5 */ bool mD5;
    /* 0x0D6 */ u8 _D6[0x02];
    /* 0x0D8 */ f32 mD8;
    /* 0x0DC */ HitSensor* mSensorDC;
    /* 0x0E0 */ bool mE0;
    /* 0x0E1 */ u8 _E1[0x0F];
};
