#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TQuat.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class AreaObj;
class CameraTargetMtx;
class HitSensor;
class ModelObj;
class RollingRockStateParent;
class RollingRockStateRoll;

class RollingRock : public LiveActor {
public:
    RollingRock(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void calcAnim();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    bool isRollingReady() const NO_INLINE;
    void exeWait();

    ModelObj* createBreakModel(MtxPtr pMtx) NO_INLINE;
    void appearBreakStart() NO_INLINE;
    void appearBreakEnd() NO_INLINE;
    void showRock() NO_INLINE;
    void hideRock() NO_INLINE;
    ModelObj* createBlurModel(MtxPtr pMtx) NO_INLINE;
    void appearBlur() NO_INLINE;
    void killBlur() NO_INLINE;
    bool onHitEnemy(HitSensor* pSender, HitSensor* pReceiver) NO_INLINE;
    static bool isNotBreakableSensor(const HitSensor* pSensor) NO_INLINE;
    static bool isBreakableSensor(const HitSensor* pSensor) NO_INLINE;
    bool isOnForceDashGround() const NO_INLINE;
    static bool calcForceDashVelocity(TVec3f* pDst, const LiveActor* pActor, f32 speed) NO_INLINE;
    static bool getForceDashStep(s32* pDst, const LiveActor* pActor, s32 defaultStep) NO_INLINE;
    static bool calcStickDir(TVec3f* pDst) NO_INLINE;
    void endBindWait() NO_INLINE;
    void endBindDamage(u32 msg, const TVec3f& rDir) NO_INLINE;
    void endBindFlip(u32 msg, const TVec3f& rDir) NO_INLINE;
    static bool isPlayerRolling() NO_INLINE;

    /* 0x90 */ ModelObj* mBlurModel;
    /* 0x94 */ ModelObj* mBreakModel;
    /* 0x98 */ RollingRockStateParent* mState;
    /* 0x9C */ s32 mArg7;
    /* 0xA0 */ TQuat4f mQuat;
    /* 0xB0 */ TPos3f mMtx;
    /* 0xE0 */ TPos3f mEffectMtx;
    /* 0x110 */ TVec3f mEffectPos;
    /* 0x11C */ TVec3f mFront;
};

class RollingRockStateBase : public ActorStateBase< RollingRock > {
public:
    RollingRockStateBase(const char* pName, RollingRock* pHost) : ActorStateBase< RollingRock >(pName, pHost) {}

    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver) {}
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
        return false;
    }
    virtual bool receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
        return false;
    }
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
        return false;
    }
};

class RollingRockStateParent : public RollingRockStateBase {
public:
    RollingRockStateParent(RollingRock* pHost);

    virtual ~RollingRockStateParent();
    virtual void appear();
    virtual void kill();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    bool isRollingReady() const;
    bool tryStartRoll() NO_INLINE;
    void followPlayer() NO_INLINE;
    bool tryEndRush() NO_INLINE;
    bool isRockModeEnd() const;
    void exeWait();
    void exeRoll();
    void endRoll() NO_INLINE;
    void exeEnd();
    void exeDamage();

    /* 0x10 */ RollingRockStateRoll* mRollState;
    /* 0x14 */ HitSensor* mItemSensor;
    /* 0x18 */ bool mIsCancel;
};

class RollingRockStateRoll : public RollingRockStateBase {
public:
    RollingRockStateRoll(RollingRock* pHost);

    virtual ~RollingRockStateRoll();
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void initFront() NO_INLINE;
    bool tryEnd() NO_INLINE;
    bool tryJump() NO_INLINE;
    bool tryDeath() NO_INLINE;
    bool tryReflect() NO_INLINE;
    bool tryLand() NO_INLINE;
    void updateVelocity() NO_INLINE;
    void updatePoseNoMoment() NO_INLINE;
    void setPlayerPose() NO_INLINE;
    void updateStep() NO_INLINE;
    void updateMoment() NO_INLINE;
    void updatePose() NO_INLINE;
    void updateEffect() NO_INLINE;
    bool isAttackEnable() const;
    void startMove() NO_INLINE;
    void changeState(s32 type) NO_INLINE;
    bool turnByStick() NO_INLINE;
    bool tryForceDash() NO_INLINE;
    bool tryPadSwing() NO_INLINE;
    bool tryTimeOver() NO_INLINE;
    void exeWait();
    void exeStart();
    void exeAppear();
    void exeMove();
    void exeAccel();
    void exeForceDash();
    void exeJump();
    bool turnByStickStart() NO_INLINE;
    bool turnByStickAppear() NO_INLINE;

    /* 0x10 */ CameraTargetMtx* mCameraTarget;
    /* 0x14 */ TVec3f mDir;
    /* 0x20 */ s32 mDashStepMax;
    /* 0x24 */ s32 mDashStep;
    /* 0x28 */ s32 mAirStep;
    /* 0x2C */ f32 mTurnRate;
    /* 0x30 */ s32 mStep;
    /* 0x34 */ f32 _34;
    /* 0x38 */ f32 mSpeed;
    /* 0x3C */ s32 mJumpBuffer;
    /* 0x40 */ f32 mStickRate;
    /* 0x44 */ bool mIsFirstRoll;
};
