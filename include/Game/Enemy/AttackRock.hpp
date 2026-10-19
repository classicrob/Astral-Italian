#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Boss/KoopaFinalBattlePos.hpp"
#include "Game/Boss/KoopaBattleFunction.hpp"
#include "Game/System/NerveExecutor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class AttackRock;
class AttackRockFinal;
class AttackRockTutorial;
class BreakModel;
class PartsModel;
class ProjmapEffectMtxSetter;
class VolumeDrawer;
class KoopaJrRobotShockWave;

// SMG2-only unit (Koopa battle rocks). Offsets are SMG2 (LiveActor = 0x90). Written from the target asm.

// Helper that lets the player ride the rock ("AutoRush" handling).
// ---- unit-local free helpers (AttackRock family; defined in the .cpp files) ----
void setupRockBinder(LiveActor* pActor) NO_INLINE;
f32 calcVelocityAngle(LiveActor* pActor, TQuat4f* pQuat) NO_INLINE;
void calcBoundVelocity(TVec3f* pOut, TQuat4f* pQuat, HitSensor* pSender, HitSensor* pReceiver, f32 speed) NO_INLINE;
void bouncePlayerAndCalcVelocity(TVec3f* pOut, TVec3f* pDir, HitSensor* pSender, HitSensor* pReceiver, f32 speed) NO_INLINE;
void updateBaseMtxFromVelocity(TPos3f* pMtx, LiveActor* pActor) NO_INLINE;
void updateProjmapEffectMtx(ProjmapEffectMtxSetter* pSetter, TQuat4f* pQuat, TVec3f* pPos) NO_INLINE;
ProjmapEffectMtxSetter* createProjmapEffectMtxSetter(LiveActor* pActor) NO_INLINE;

class AttackRockRide {
public:
    AttackRockRide(LiveActor* pHost);
    void setHostZDir(const TVec3f* pVec);
    void initRush();
    void updateRush();
    bool tryReceiveRideMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    void enableRide() NO_INLINE;
    void endRide() NO_INLINE;

    /* 0x00 */ LiveActor* mHost;
    /* 0x04 */ TQuat4f mQuat0;
    /* 0x14 */ TQuat4f mQuat1;
    /* 0x24 */ TQuat4f mQuat2;
    /* 0x34 */ TVec3f mHostZDir;
    /* 0x40 */ TVec3f mRidePos;
    /* 0x4C */ TVec3f mPlayerMove;
    /* 0x58 */ TVec3f mDrift;
    /* 0x64 */ TVec3f mStartDir;
    /* 0x70 */ TVec3f mTargetDir;
    /* 0x7C */ TVec3f mRotAxis;
    /* 0x88 */ f32 mRotAngle;
    /* 0x8C */ f32 mRotAngleSum;
    /* 0x90 */ f32 mStartDistance;
    /* 0x94 */ s32 mRushFrames;
    /* 0x98 */ s32 mRushStep;
    /* 0x9C */ bool mIsRush;
    /* 0x9D */ bool mIsEnable;
};

// State of the rock while it is held/floating by Koopa.
class AttackRockStateHold : public ActorStateBase< AttackRock > {
public:
    bool isPowerOrFloatEnd();
    bool isFloating();
    void updateHoldPosition();
    void exeHoldFloat();
    void exeHoldPower();
    void exeHoldFloatEnd();
    void exeHoldFallBreak();
    AttackRockStateHold(AttackRock*, s32);
    virtual ~AttackRockStateHold();

    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);


    /* 0x10 */ AttackRockRide* mRide;
    /* 0x14 */ BreakModel* mBreakModel;
    /* 0x18 */ TVec3f mStartPos;
    /* 0x24 */ TVec3f mUpVec;
    /* 0x30 */ f32 mRate;
    /* 0x34 */ s32 mRockNum;
};

// State of the rock while it falls and creates the shockwave.
class AttackRockStateFall : public ActorStateBase< AttackRock > {
public:
    void updateLandSensorRadius();
    void updateFallBezierVelocity();
    void startLandDemo();
    void exeFallAppear();
    void exeFallWait();
    void exeFallLanding();
    AttackRockStateFall(AttackRock*, s32);
    virtual ~AttackRockStateFall();

    virtual void appear();
    virtual void kill();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);


    /* 0x10 */ KoopaJrRobotShockWave* mBreakModel;
    /* 0x14 */ VolumeDrawer* mVolumeDrawer;
    /* 0x18 */ TVec3f mFallPos;
    /* 0x24 */ TVec3f mStartPos;
    /* 0x30 */ TVec3f mLandPos;
    /* 0x3C */ TVec3f mLandNormal;
    /* 0x48 */ bool mIsNear;
};

class AttackRock : public LiveActor {
public:
    f32 getRockBaseRadius() const NO_INLINE;
    bool isOuterState() const NO_INLINE;
    bool isRockFollowValid(s32) const;
    void startRockBreak();
    void exeRockBound();
    void exeRockBreak();
    AttackRock(const char* pName);

    virtual ~AttackRock();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);



    /* 0x90 */ TPos3f mMtx90;
    /* 0xC0 */ TPos3f mMtxC0;
    /* 0xF0 */ TQuat4f mQuatF0;
    /* 0x100 */ TQuat4f mQuat100;
    /* 0x110 */ BreakModel* mBreakModel;
    /* 0x114 */ LiveActor* mDemoObj;
    /* 0x118 */ ProjmapEffectMtxSetter* mProjmapEffectMtxSetter;
    /* 0x11C */ AttackRockStateFall* mStateFall;
    /* 0x120 */ AttackRockStateHold* mStateHold;
    /* 0x124 */ s32 mRockNum;
    /* 0x128 */ bool mIsOnPlayer;
};

class AttackRockFinal : public LiveActor {
public:
    bool isFollowValidFinal(s32) const;
    void setPositionFromAxes(f32 a, f32 b);
    void exeFinalWait();
    void exeFinalFall();
    void exeFinalFloat();
    void exeFinalBound();
    void exeFinalBreak();
    AttackRockFinal(const char*);

    virtual ~AttackRockFinal();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    /* 0x90 */ ProjmapEffectMtxSetter* mProjmapEffectMtxSetter;
    /* 0x94 */ BreakModel* mBreakModel;
    /* 0x98 */ AttackRockRide* mRide;
    /* 0x9C */ TPos3f mMtx9C;
    /* 0xCC */ TQuat4f mQuatCC;
    /* 0xDC */ TQuat4f mQuatDC;
    /* 0xEC */ TVec3f mStartPos;
    /* 0xF8 */ TVec3f mAxisF8;
    /* 0x104 */ TVec3f mAxis104;
    /* 0x110 */ s32 mWaitStep;
    /* 0x114 */ bool mIsOnPlayer;
};

class AttackRockTutorial : public LiveActor {
public:
    bool isFollowValidTutorial(s32) const;
    void startDead();
    void exeTutorialWait();
    void exeTutorialFloatUp();
    void exeTutorialFloat();
    void exeTutorialBound();
    void exeTutorialBreak();
    AttackRockTutorial(const char*);

    virtual ~AttackRockTutorial();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    /* 0x90 */ TPos3f mMtx90;
    /* 0xC0 */ ProjmapEffectMtxSetter* mProjmapEffectMtxSetter;
    /* 0xC4 */ BreakModel* mBreakModel;
    /* 0xC8 */ PartsModel* mHillModel;
    /* 0xCC */ ActorCameraInfo* mCameraInfo;
    /* 0xD0 */ AttackRockRide* mRide;
    /* 0xD4 */ TQuat4f mQuatD4;
    /* 0xE4 */ TVec3f mStartPos;
    /* 0xF0 */ TVec3f mUpVec;
    /* 0xFC */ f32 mRate;
    /* 0x100 */ s32 mTouchCount;
    /* 0x104 */ bool mTouched;
    /* 0x105 */ bool mUseCamera;
};
