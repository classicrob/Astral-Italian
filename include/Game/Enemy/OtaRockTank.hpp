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

    JointController* createTankJointController(const char*, bool (OtaRockTank::*)(TPos3f*, const JointControllerInfo&));

    // Rail movement, projectile emission, and joint callbacks.
    f32 getTankMinSpeed();
    f32 getTankMaxSpeed();
    f32 getTankRailBrakeDistance();
    bool tryReachTankRailPoint();
    void moveTankAlongRail();
    void moveTankTowardRailDirection(const TVec3f&, f32);
    void syncTankRailCoordinate();
    void updateTankEaseOutSpeed(s32, f32);
    void updateTankBrake(f32);
    void updateTankBrakeSpeed(s32, f32, f32);
    void accelerateTankMove();
    void approachTankCruiseSpeed();
    void decelerateTankWheels();
    void accelerateTankWheels();
    void approachTankWheelSpeed();
    s32 countFreeTankFireBalls();
    CocoNutBall* findFreeTankCoconutBall();
    FireBall* findFreeTankFireBall();
    ScatterFireBall* findFreeTankScatterBall();
    bool tryFireTankVolley();
    void fireTankSideBatteries();
    void fireTankMouth();
    bool isTankMeteorCoconut(u32, s32);
    void fireTankMeteorForward(u32, s32);
    void fireTankMeteorAtTarget(u32, s32);
    void fireTankFlame(const TVec3f&, const TVec3f&);
    bool calcTankLeftWheelJoint(TPos3f* pMtx, const JointControllerInfo&);
    bool calcTankRightWheelJoint(TPos3f* pMtx, const JointControllerInfo&);
    NO_INLINE bool calcTankWheelJoint(TPos3f* pMtx, const JointControllerInfo&);
    bool calcTankChiefJoint(TPos3f* pMtx, const JointControllerInfo&);
    bool isTankDashing();
    bool canTankFireAtRailPoint();
    void startTankRunAnimations();
    bool isTankChiefTrackingPlayer();
    bool tryStartTankDemo();
    void killTankProjectiles();

public:
    /* 0x90 */ BreakModel* mBreakModel;
    /* 0x94 */ ActorCameraInfo* mActorCameraInfo;
    /* 0x98 */ ActorCameraInfo* mStartDemoCameraInfo;
    /* 0x9C */ ActorCameraInfo* mDownDemoCameraInfo;
    /* 0xA0 */ TVec3f mCameraRegisterPosition;
    /* 0xAC */ bool mHasPowerStar;
    /* 0xB0 */ s32 mLife;
    /* 0xB4 */ OtaRockChief* mChief;
    /* 0xB8 */ JointController* mLeftWheelJointController;
    /* 0xBC */ JointController* mRightWheelJointController;
    /* 0xC0 */ JointController* mChiefJointController;
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
    /* 0x120 */ u8 mCanFireAtRailPoint;
    /* 0x121 */ u8 mVolleyStarted;
    /* 0x122 */ u8 mUnusedFlag;
    /* 0x124 */ s32 mFireTimer;
    /* 0x128 */ s32 mThrowNum;
    /* 0x12C */ s32 mMeteorStep;
    /* 0x130 */ f32 mSpeedStart;
};
