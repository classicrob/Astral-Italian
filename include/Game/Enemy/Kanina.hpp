#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Player/YoshiFunction.hpp"

class AnimationRandomPlayer;
class BenefitItemObj;
class JointRumbler;
class YoshiLockOnTarget;
class KaninaYoshiState;

namespace MR {
    bool isMsgYoshiStartTongue(u32);
    bool isMsgYoshiLoopTongue(u32);
    bool isMsgYoshiStartEaten(u32);
    bool isMsgYoshiCancelEaten(u32);
    bool isMsgYoshiReleaseEaten(u32);
};  // namespace MR

class Kanina : public LiveActor {
public:
    enum KaninaType {
        KaninaType_Blue = 0,
        KaninaType_Red = 1,
    };

    enum SeVersion {
        SeVersion_NORMAL = 0,
        SeVersion_WATER = 1,
    };

    Kanina(const char*);

    virtual ~Kanina();
    virtual void init(const JMapInfoIter& rIter);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void appearItemTrample();
    bool tryAttack(HitSensor*, HitSensor*);
    bool tryAttackPlayer(HitSensor*, HitSensor*);
    bool tryPushEach(HitSensor*, HitSensor*);
    bool tryPushEachEnemy(HitSensor*, HitSensor*);
    bool isNotYoshiNerve() const;
    bool receiveMsgYoshi(u32, HitSensor*, HitSensor*);
    bool receiveTrample(HitSensor*, HitSensor*);
    bool receivePunch(HitSensor*, HitSensor*);
    bool receiveFireBall(HitSensor*, HitSensor*);
    void startDamageFireBall(HitSensor*, HitSensor*);
    void doDamageFireBall(HitSensor*, HitSensor*);
    void startRun();
    void startRunAwayLevelSound();
    bool receiveHipDrop(HitSensor*, HitSensor*);
    bool receiveInvincibleAttack(HitSensor*, HitSensor*);
    void updateSeVersion();
    bool isStateStayOnGround() const;
    void initForType(const JMapInfoIter&, KaninaType);
    bool isPlayerBackward(f32) const;
    bool isStatePossibleToAttack() const;
    bool isStateBlink() const;
    bool isYoshiNerve() const;
    f32 getGravityAccel() const;
    void updateMovement();
    void updateTimers();
    bool tryFindPlayer();
    bool tryHitWall();
    bool tryAvoidFall();
    bool tryAvoidWaterAhead();
    bool tryAvoidFallNextMove();
    bool tryDigOrTurn();
    bool tryPointing();
    bool tryYoshiStartTongue();
    bool tryDig();
    bool tryTurn();
    void getWalkDirection(TVec3f*);
    f32 getRandomSign();

    void exeAppear();
    void exeWait();
    void exeWalk();
    void exeRunAway();
    void exeRunAwayReboundDirection();
    void exeRunAwayBreak();
    void exeAttack();
    void exeVauntAttackSuccess();
    void exeHitWall();
    void exeReboundEach();
    void exeGuard();
    void exeGuardEnd();
    void exeDamageFireBall();
    void exeDig();
    void exeWaitUnderGround();
    void exeFindPlayer();
    void exeTurn();
    void exeTurnEnd();
    void exePointing();
    void endPointing();
    void exeShot() {}
    void exeTongue() {}
    void exeEaten();

    bool isStateGuard() const;
    bool isStateDamageNoFireBall() const;
    bool isBindedGroundWaterBottom() const;

    inline f32 getFloatDependentType(f32 fRed, f32 fBlue) {
        return mType == KaninaType_Red ? fRed : fBlue;
    }

    /* 0x090 */ KaninaType mType;
    /* 0x094 */ KaninaYoshiState* mState;
    /* 0x098 */ JointRumbler* mJointRumbler;
    /* 0x09C */ BenefitItemObj* mKinokoOneUp;
    /* 0x0A0 */ AnimationRandomPlayer* mAnimationRandomPlayer;
    /* 0x0A4 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0xA8 */ TVec3f mHomePos;
    /* 0xB4 */ TQuat4f mPoseQuat;
    /* 0xC4 */ u32 mRunawayTime;
    /* 0xC8 */ u32 mRunawayBreakTime;
    /* 0xCC */ f32 mRunawaySideSign;
    /* 0xD0 */ TVec3f mReboundDirection;
    /* 0xDC */ TVec3f mFront;
    /* 0xE8 */ s32 mWalkCount;
    /* 0x0EC */ s32 mGuardTimer;
    /* 0x0F0 */ s32 mTimer2;
    /* 0x0F4 */ bool mIsNoDig;
};
