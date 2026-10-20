#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Util/FixedPosition.hpp"

class BossBussun;
class BossBussunShot;
class BossBussunSequencer;
class ProjmapEffectMtxSetter;
class AnimScaleController;
class JointController;
struct JointControllerInfo;
class WalkerStateBindStarPointer;
class ActorCameraInfo;
class HitSensor;
class Nerve;

// SMG2-only boss "Bussun" family (no Petari counterpart). Layouts from the target asm.

class BossBussunStateSleep : public ActorStateBase< BossBussun > {
public:
    BossBussunStateSleep(BossBussun*);

    virtual ~BossBussunStateSleep();
    virtual void appear();

    bool tryWake();
    bool receiveOtherMsg(u32);
    void exeSleep();
    void exeSleepReaction();
};

class BossBussunStateRecover : public ActorStateBase< BossBussun > {
public:
    BossBussunStateRecover(BossBussun*);

    virtual ~BossBussunStateRecover();
    virtual void appear();

    void exeRecover();
};

class BossBussunStateDash : public ActorStateBase< BossBussun > {
public:
    BossBussunStateDash(BossBussun*);

    virtual ~BossBussunStateDash();
    virtual void appear();
    virtual void control();

    bool tryStartDash();
    void exeStart();
    void exeDash();
    void exeDashEnd();

    /* 0x10 */ s32 mCoolTime;
};

class BossBussunStateDemo : public ActorStateBase< BossBussun > {
public:
    BossBussunStateDemo(BossBussun*, const char*, bool);

    virtual ~BossBussunStateDemo();
    virtual void appear();

    void exeDemo();
    void exeWaitEnd();

    /* 0x10 */ const char* mDemoName;
    /* 0x14 */ bool mIsPowerStarDemo;
};

class BossBussunStateDamage : public ActorStateBase< BossBussun > {
public:
    BossBussunStateDamage(BossBussun*);

    virtual ~BossBussunStateDamage();
    virtual void appear();

    void exeDamage();
    void updateDamagePos();

    /* 0x10 */ TVec3f mPosA;
    /* 0x1C */ TVec3f mPosB;
    /* 0x28 */ TVec3f mPosC;
};

class BossBussunStateShot : public ActorStateBase< BossBussun > {
public:
    BossBussunStateShot(BossBussun*);

    virtual ~BossBussunStateShot();
    virtual void appear();
#ifndef GALAXY_NATIVE
    virtual void kill();  // not in the DOL vtable (slot is ActorStateBaseInterface::kill)
#endif

    void exeShotSign();
    void exeShot();
    void setShotCountMax(s32);
    void killAll();

    /* 0x10 */ DeriveActorGroup< BossBussunShot >* mShotGroup;
    /* 0x14 */ FixedPosition* mFixedPos;
    /* 0x18 */ s32 mShotCount;
    /* 0x1C */ s32 mShotCountMax;
};

class BossBussunStateChangeSide : public ActorStateBase< BossBussun > {
public:
    BossBussunStateChangeSide(BossBussun*);

    virtual ~BossBussunStateChangeSide();
    virtual void appear();

    void exeTurnToEdge();
    void exeMoveToEdge();
    void exeTurnFromCenter();
    void exeMoveFromCenter();
    void exeFlip();
    void exeMoveToHome();
    void exeMoveAcross();

    /* 0x10 */ TVec3f mDirFromCenter;
    /* 0x1C */ TVec3f mAxis;
    /* 0x28 */ TVec3f mUpDir;
    /* 0x34 */ TVec3f mStartPos;
    /* 0x40 */ f32 mHeight;
    /* 0x44 */ bool mIsUpSide;
};

class BossBussunShot : public LiveActor {
public:
    BossBussunShot(const char*);

    virtual ~BossBussunShot();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);

    void exeFly();
    void exeHold();
    void exeLandWait();
    void exeLand();
    void exeBreak();
    void shoot(const TVec3f&, const TVec3f&);
    bool calcJointMtx(TPos3f*, const JointControllerInfo&);

    /* 0x090 */ AnimScaleController* mAnimScale;
    /* 0x094 */ WalkerStateBindStarPointer* mBindState;
    /* 0x098 */ HitSensor* mHoldSensor;
    /* 0x09C */ JointController* mJointCtrl;
    /* 0x0A0 */ TQuat4f mQuatA;
    /* 0x0B0 */ TQuat4f mQuatB;
    /* 0x0C0 */ TVec3f _C0;
    /* 0x0CC */ TVec3f mMoveDir;
    /* 0x0D8 */ TVec3f mHoldPos;
    /* 0x0E4 */ TVec3f mHoldVel;
    /* 0x0F0 */ s32 _F0;
    /* 0x0F4 */ s32 mGroundTimer;
    /* 0x0F8 */ s32 mStep;
};

class BossBussunSequencer {
public:
    BossBussunSequencer(BossBussun*, const JMapInfoIter&);

    void initCameraState();
    void startCamera();
    void endCamera();
    void updateCamera();
    void calcFrameAxes(TVec3f*, TVec3f*);
    f32 calcPlayerDot() const;
    bool isNearHeight(f32) const;

    /* 0x00 */ BossBussun* mHost;
    /* 0x04 */ TVec3f mCenterPos;
    /* 0x10 */ TVec3f mUpVec;
    /* 0x1C */ TVec3f mSideVec;
    /* 0x28 */ TVec3f mCamFront;
    /* 0x34 */ TVec3f mCamUp;
    /* 0x40 */ TVec3f mCamPos;
    /* 0x4C */ bool mIsNear;
};

namespace BossBussunFunction {
    void setBinderExceptPlayer(LiveActor*);
};  // namespace BossBussunFunction

class BossBussun : public LiveActor {
public:
    BossBussun(const char*);

    virtual ~BossBussun();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    // nerve executors
    void exeStart();
    void exeWait();
    void exeTurn();
    void exeMove();
    void exeFind();
    void exeDash();
    void exeDamage();
    void exeDemoDown();

    void startSleep();
    f32 getDashDistanceLimit() const;
    f32 getAttackDistanceLimit() const;
    void calcNearestHomePos(TVec3f*);
    bool checkSensorPlayerInFront(HitSensor*, HitSensor*);
    void updateVelocityOnGround();
    bool isPlayerAttackable();
    bool tryStartAttack();
    void storePlayerPos() NO_INLINE;
    void resetBodyOffset();
    void resetAnimScale();
    bool isEnableAttack() const;
    bool isSleeping() const NO_INLINE;
    bool isEnableDamage() const;
    bool isAngry() const;
    bool isAttackingNerve() const;
    void resetToHomePos();

    /* 0x090 */ ProjmapEffectMtxSetter* mProjmapEffect;
    /* 0x094 */ BossBussunSequencer* mSequencer;
    /* 0x098 */ ActorCameraInfo* mCameraInfo;
    /* 0x09C */ BossBussunStateSleep* mSleepState;
    /* 0x0A0 */ BossBussunStateShot* mShotState;
    /* 0x0A4 */ BossBussunStateDash* mDashState;
    /* 0x0A8 */ AnimScaleController* mAnimScale;
    /* 0x0AC */ TVec3f mBodyUp;
    /* 0x0B8 */ TVec3f mGroundPos;
    /* 0x0C4 */ TVec3f mTargetPos;
    /* 0x0D0 */ TVec3f mFrontDir;
    /* 0x0DC */ TVec3f mHomePos;
    /* 0x0E8 */ TVec3f mUpVec;
    /* 0x0F4 */ TVec3f mCenterPos;
    /* 0x100 */ s32 mStepCount;
    /* 0x104 */ f32 mBodyOffset;
    /* 0x108 */ s32 mHitCount;
    /* 0x10C */ u8 mSeVersion;
    /* 0x10D */ u8 mSeVersionNext;
};
