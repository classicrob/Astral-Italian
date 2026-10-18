#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ModelObj;
class AnimScaleParam;
class AnimScaleController;
class PartsModel;

class YoshiLockOnTarget;

namespace YoshiFunction {
    void tryYoshiProjectile(YoshiLockOnTarget*);
};

extern "C" void fn_804221D0();
extern "C" void fn_80422250();

namespace MR {

    bool isMsgYoshiStartTongue(u32);
    bool isMsgYoshiLoopTongue(u32);
    bool isMsgYoshiStartEaten(u32);
    bool isMsgYoshiCancelEaten(u32);
    bool isMsgYoshiReleaseEaten(u32);
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, MtxPtr, const TVec3f&);
};

class HomingKiller : public LiveActor {
public:
    enum Type { Type_HomingKiller = 0, Type_Torpedo = 1, Type_MagnumKiller = 2 };

    HomingKiller(const char* = "ホーミングキラー");

    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void appear(const TVec3f&, const TVec3f&);
    bool isNerveAppear() const;
    bool isMoveStart() const;
    void setTypeMagnumKiller();
    void setChaseRotateSpeed(f32);
    void initSpeed();
    void setType(s32) NO_INLINE;
    void setMovementType(s32);
    bool isNormalNerve() const;
    bool isYoshiHold() const;
    bool isShotEnd() const;
    bool isEaten() const;
    bool isBreakByEnemyAttack(u32, HitSensor*);
    void setChaseStartEndDistance(f32, f32);
    void initMapToolInfo(const JMapInfoIter&);
    void calcInitPosture();
    void updateVelocity();
    void updateRotateZ(const TVec3f&);
    bool processMove();
    bool processChase();

    bool isChaseStart() const;
    bool tryChaseStart();
    bool tryFreeze(const Nerve*);
    bool tryBindedBreak();
    bool tryToExplosion(HitSensor* pSender, HitSensor* pReceiver);
    void sendMsgExplosionToNearActor();

    bool isUpdateChaseFrontVec(const TVec3f&) const;
    void calcFrontVecToTarget(TVec3f*) const;

    bool isValidShowModel() const;
    bool isWaterBreak() const;

    void updateBaseMtxNoRotateZ();
    void startMoveLevelSound(bool);
    void setBckRate(f32, bool);
    bool isChasing() const;

    void exeAppear();
    void exeMoveStart();
    void exeMove();
    void exeChaseStart();
    void exeChase();
    void exeFreeze();
    void exeBreak();
    void exeGoToTarget();
    void exeTongue() {}
    void endEaten() {
        fn_804221D0();
        fn_80422250();
    }
    void exeEaten();
    void exeShot();

    inline f32 getScale() {
        return mType == Type_MagnumKiller ? 4.0f : 1.0f;
    }

    void setBasePosAndFront(const TVec3f& pos, const TVec3f& front) {
        mBasePos.set(pos);
        mBaseFront.set(front);
    }

    bool isGravityIgnored() const {
        return mIsLinearShot || mUseFullSightAngle;
    }

    /* 0x90 */ Type mType;
    /* 0x94 */ f32 mSpeed;
    /* 0x98 */ f32 mChaseStartDist;
    /* 0x9C */ f32 mChaseEndDist;
    /* 0xA0 */ f32 mChaseRotateSpeed;
    /* 0xA4 */ f32 mChaseStartAngle;
    /* 0xA8 */ TVec3f mFront;
    /* 0xB4 */ TVec3f mUp;
    /* 0xC0 */ TVec3f mBasePos;
    /* 0xCC */ TVec3f mBaseFront;
    /* 0xD8 */ TVec3f mBaseUp;
    /* 0xE4 */ TPos3f mBaseMtx;
    /* 0x114 */ const Nerve* mUnfreezeNerve;
    /* 0x118 */ s32 mChaseInvalidTime;
    /* 0x11C */ s32 mMoveTime;
    /* 0x120 */ TPos3f mEffectMtx;
    /* 0x150 */ HitSensor* mTargetSensor;
    /* 0x154 */ bool mDisableChase;
    /* 0x155 */ bool mIsLinearShot;
    /* 0x156 */ bool mUseFullSightAngle;
    /* 0x158 */ PartsModel* mPropeller;
    /* 0x15C */ ModelObj* mTorpedoLight;
    /* 0x160 */ s32 mShotBindTime;
    /* 0x164 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0x168 */ s32 mMovementType;
    /* 0x16C */ s32 mForceKillTime;
    /* 0x170 */ s32 mChaseInvalidFrame;
    /* 0x174 */ s32 mShotTime;
    /* 0x178 */ AnimScaleParam* mScaleParam;
    /* 0x17C */ AnimScaleController* mScaleController;
};

class HomingKillerLauncher : public LiveActor {
public:
    // SMG2-only functions (original names unknown; named by address)
    HomingKiller* fn_80198540();
    HomingKillerLauncher(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeAppearKiller();
    void exeDeadKiller();

    /* 0x90 */ HomingKiller* mKillers;
    /* 0x94 */ s32 mKillerNum;
    /* 0x98 */ s32 mAppearInterval;
};
