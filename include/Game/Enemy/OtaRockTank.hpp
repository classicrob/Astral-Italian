#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JointController.hpp"

class BreakModel;
class ActorCameraInfo;
class AnimScaleController;
class CocoNutBall;
class FireBall;
class ScatterFireBall;
class FixedPosition;
class OtaRockChief;

/// @brief OtaRockTank (tank boss that rides a rail and fires coconuts and fire balls). SMG2-only.
class OtaRockTank : public LiveActor {
public:
    OtaRockTank(const char* pName);

    virtual ~OtaRockTank();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    // Nerve executors (names from the actions they start).
    void exeRunAccel();       // fn_801DE060
    void exeRunKeep();        // fn_801DE120
    void exeBrakeA();         // fn_801DE1A0
    void exeRunRail();        // fn_801DE2A0
    void exeStartDemo();      // fn_801DE370
    void exeRunStart();       // fn_801DE490
    void exeRun();            // fn_801DE5D0
    void exeFireAttack();     // fn_801DE6F0
    void exeBrakeB();         // fn_801DE8A0
    void exeTurnAttack();     // fn_801DE9D0
    void exeRunFukashi();     // fn_801DEB20
    void exeMeteor();         // fn_801DEC30
    void exeDashStart();      // fn_801DEDD0
    void exeDash();           // fn_801DF030
    void exeDashBrake();      // fn_801DF2B0
    void exeDamage();         // fn_801DF580
    void exeDownDemo();       // fn_801DF780
    void exeExplosion();      // fn_801DF870

    JointController* fn_801DDD80(const char*, bool (OtaRockTank::*)(TPos3f*, const JointControllerInfo&));

    // Unnamed helpers, named by address.
    f32 fn_801DFBB0();
    f32 fn_801DFC60();
    f32 fn_801DFD10();
    bool fn_801DFDC0();
    void fn_801DFE20();
    void fn_801DFF10(const TVec3f&, f32);
    void fn_801E0170();
    void fn_801E01F0(s32, f32);
    void fn_801E0340(f32);
    void fn_801E0430(s32, f32, f32);
    void fn_801E04B0();
    void fn_801E0540();
    void fn_801E05E0();
    void fn_801E0630();
    void fn_801E0680();
    s32 fn_801E0700();
    CocoNutBall* fn_801E0770();
    FireBall* fn_801E07E0();
    ScatterFireBall* fn_801E0850();
    bool fn_801E08C0();
    void fn_801E0940();
    void fn_801E0A90();
    bool fn_801E0CA0(u32, s32);
    void fn_801E0D90(u32, s32);
    void fn_801E0E80(u32, s32);
    void fn_801E0F80(const TVec3f&, const TVec3f&);
    bool fn_801E1010(TPos3f* pMtx, const JointControllerInfo&);
    bool fn_801E1020(TPos3f* pMtx, const JointControllerInfo&);
    NO_INLINE bool fn_801E1030(TPos3f* pMtx, const JointControllerInfo&);
    bool fn_801E1090(TPos3f* pMtx, const JointControllerInfo&);
    bool fn_801E1170();
    bool fn_801E1220();
    void fn_801E12C0();
    bool fn_801E1320();
    bool fn_801E1490();
    void fn_801E1550();

public:
    /* 0x90 */ BreakModel* mBreakModel;
    /* 0x94 */ ActorCameraInfo* mActorCameraInfo;
    /* 0x98 */ ActorCameraInfo* mAnimCameraInfo;
    /* 0x9C */ ActorCameraInfo* mAnimCameraInfo2;
    /* 0xA0 */ TVec3f mPos;
    /* 0xAC */ bool mHasPowerStar;
    /* 0xB0 */ s32 mLife;
    /* 0xB4 */ OtaRockChief* mChief;
    /* 0xB8 */ JointController* mJointCtrl0;
    /* 0xBC */ JointController* mJointCtrl1;
    /* 0xC0 */ JointController* mJointCtrl2;
    /* 0xC4 */ f32 mWheelRot;
    /* 0xC8 */ s32 mShotIndex;
    /* 0xCC */ u32 mMeteorCount;
    /* 0xD0 */ s32 mMeteorRandom;
    /* 0xD4 */ CocoNutBall* mCocoNutBalls;
    /* 0xD8 */ FireBall* mFireBalls;
    /* 0xDC */ ScatterFireBall* mScatterBalls;
    /* 0xE0 */ FixedPosition** mFixedPositions;
    /* 0xE4 */ TVec3f* mTargetPoses;
    /* 0xE8 */ TVec3f mRailClipPos;
    /* 0xF4 */ TVec3f mRailDirection;
    /* 0x100 */ TVec3f mPrevRailDirection;
    /* 0x10C */ f32 mWheelSpeed;
    /* 0x110 */ u8 mDamageMask;
    /* 0x114 */ s32 mCurrentRailPointNo;
    /* 0x118 */ s32 mTargetRailPointNo;
    /* 0x11C */ f32 mSpeed;
    /* 0x120 */ u8 mUnk120;
    /* 0x121 */ u8 mUnk121;
    /* 0x122 */ u8 mUnk122;
    /* 0x124 */ s32 mFireTimer;
    /* 0x128 */ s32 mThrowNum;
    /* 0x12C */ s32 mMeteorStep;
    /* 0x130 */ f32 mSpeedStart;
};
