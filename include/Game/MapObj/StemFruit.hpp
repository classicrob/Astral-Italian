#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "math_types.hpp"

class StemFruitChild;
class FixedPosition;
class YoshiLockOnTarget;

namespace YoshiFunction {
    void tryYoshiEat(YoshiLockOnTarget*);
    void tryYoshiPull(YoshiLockOnTarget*);
    bool tryYoshiPushEat();
}

namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, MtxPtr, const TVec3f&);
    bool isMsgYoshiAutoEatStart(u32);
    bool isMsgYoshiAutoEatEnd(u32);
}

// Per-fruit parameter block returned by StemFruit::getParam() (virtual slot 0xC0, implemented by the
// concrete fruit classes in other TUs). Offsets read from StemFruit / StemFruitChild code.
struct StemFruitParam {
    /* 0x00 */ const char* mModelName;
    /* 0x04 */ const char* mChildModelName;
    /* 0x08 */ u8 mFlag08;
    /* 0x0C */ s32 mRespawnStep;
    /* 0x10 */ u8 mFlag10;
    /* 0x11 */ u8 mFlag11;
    /* 0x12 */ u8 mFlag12;
    /* 0x14 */ s32 mStarPointerRadiusS32;
    /* 0x18 */ f32 mStarPointerOffsetY;
    /* 0x1C */ f32 mHitSensorRadius;
    /* 0x20 */ f32 mTimerStep;
    /* 0x24 */ u8 mFlag24;
    /* 0x2C */ const char* mEffectName;
};

// SMG2-only unit (no Petari source). Layout and virtual slot order from the
// __vt__9StemFruit table (0x806AFAA0). Virtual slots 0x70..0xD4 are StemFruit's own.
class StemFruit : public LiveActor {
public:
    StemFruit(const char* pName);

    virtual ~StemFruit();                                     // slot 0x08
    virtual void init(const JMapInfoIter&);                   // slot 0x0C
    virtual void control();                                   // slot 0x50
    virtual void exeWait();                                   // slot 0x70
    virtual void exeTrampled();                               // slot 0x74
    virtual void exeSpin();                                   // slot 0x78
    virtual void exeShake();                                  // slot 0x7C
    virtual void exeGet();                                    // slot 0x80
    virtual void exeComeOff();                                // slot 0x84
    virtual void exePull();                                   // slot 0x88
    virtual void exePullEnd();                                // slot 0x8C
    virtual void exeComeOffAlt();                             // slot 0x90
    virtual bool tryStartTrampled();                          // slot 0x94
    virtual bool tryStartSpin();                              // slot 0x98
    virtual bool tryStartShake();                             // slot 0x9C
    virtual void setNerveGet();                               // slot 0xA0
    virtual void onGetSwitch();                               // slot 0xA4, fn_8031C420
    virtual bool tryStartComeOff();                           // slot 0xA8, fn_8031C4A0
    virtual bool tryStartPull();                              // slot 0xAC, fn_8031C530
    virtual bool tryStartPullEnd();                           // slot 0xB0, fn_8031C590
    virtual bool tryStartComeOffAlt();                        // slot 0xB4, fn_8031C5E0
    virtual void setNerveWait();                              // slot 0xB8, fn_8031C640
    virtual void endPull();                                   // slot 0xBC, fn_8031C650
    virtual const StemFruitParam* getParam() = 0;             // slot 0xC0, zero entry in this table (pure)
    virtual void updateChildState();                          // slot 0xC4, fn_8031C6C0
    virtual bool isNerveActive() const;                       // slot 0xC8, fn_8031C780
    virtual void slot0CC();                                   // slot 0xCC, fn_8031BE50 (empty)
    virtual void onEat();                                     // slot 0xD0
    virtual void onEatEnd();                                  // slot 0xD4, fn_80293DE0 (other TU)

    /* 0x090 */ StemFruitChild* mChild;
    /* 0x094 */ s32 mTimer;
};

// Child object owned by StemFruit (allocated as 0xBC bytes, LiveActor based).
// Its vtable (__vt__ at 0x806AFA30, 0x70 bytes) overrides appear/calcAndSetBaseMtx/sensor handlers.
class StemFruitChild : public LiveActor {
public:
    StemFruitChild(StemFruit* pParent, const JMapInfoIter& rIter);

    virtual void appear();                                                   // slot 0x2C, fn_8031CAC0
    virtual void calcAndSetBaseMtx();                                        // slot 0x54, fn_8031D780
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);     // slot 0x5C, fn_8031D0C0
    virtual bool receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver);   // slot 0x60, fn_8031D170
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver); // slot 0x64, fn_8031D1D0
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);       // slot 0x6C, fn_8031D450

    void exeAppear();                                                        // nerve 0x455C, fn_8031CB20
    void exeWait();                                                          // nerve 0x4560, fn_8031CC30
    void exeNearPlayer();                                                    // nerve 0x4564, fn_8031CD00
    void exeGravity();                                                       // nerve 0x4568, fn_8031CDC0
    void exeBreak();                                                         // nerve 0x456C, fn_8031CE10
    void exeHipDropCrash();                                                  // nerve 0x4570, fn_8031CEA0
    void updateSensorOffset();                                               // fn_8031D9F0 (0xBC)

    /* 0x090 */ StemFruit* mParent;
    /* 0x094 */ FixedPosition* mFixedPos;
    /* 0x098 */ TVec3f mBaseScale;
    /* 0x0A4 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0x0A8 */ u32 mPad0A8;
    /* 0x0AC */ f32 mCycleTimer;
    /* 0x0B0 */ TVec3f mPullPos;
};
