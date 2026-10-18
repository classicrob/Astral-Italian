#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class ActorJointCtrl;
class HitSensor;
class Nerve;

/// @brief Big rolling bull boss (SMG2-only, "King Tossin"). Charges at the player, slides when it misses,
/// and takes damage when hit from behind (spin or a rock). mLife counts down from 4.
class KingTossin : public LiveActor {
public:
    KingTossin(const char* pName);
    virtual ~KingTossin();

    virtual void init(const JMapInfoIter& rIter);
    virtual void kill();
    virtual void calcAndSetBaseMtx();
    virtual void control();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeWaitStart();
    void exeDemoOpening();
    void exeDemoPowerUp();
    void exeDemoDown();
    void exeAttack();
    void exeDamage();
    void exeDamageSecond();
    void exeChanceSliding();
    void exeChanceEnd();
    void exeRun();
    void exeRolling();
    void exeRollingLv2();
    void exeSliding();
    void exeDownRecover();
    void exeGuard();
    void exeGuardReturn();
    void exeRunRail();
    void exeRollingStart();
    void exeRollingEnd();
    void exeRollingAttack();
    void exeSpinReaction();
    void exeTurn();
    void exeSearch();
    void exeChase();
    void exeBrake();
    void exeTurnToPlayer();
    void exeStandby() {}

    void updateCameraTarget() NO_INLINE;
    bool tryEndTargetInSight();
    bool isFrontToSensor(HitSensor* pSensor);
    bool isEnableAttack();
    bool isEnableReceiveAttack();
    bool isRolling();
    bool isTargetInSight(f32 angle, f32 distance);
    const Nerve* getWaitNerve();
    const Nerve* getRollingNerve();
    bool deleteBrakeSmoke();
    void resetPosition();

    /* 0x90 */ ActorJointCtrl* mJointCtrl;
    /* 0x94 */ TVec3f mFront;
    /* 0xA0 */ TVec3f mMoveDir;
    /* 0xAC */ f32 mSlideSpeed;
    /* 0xB0 */ s32 mLife;
    /* 0xB4 */ s32 mRollCount;
    /* 0xB8 */ s32 mTimer;
    /* 0xBC */ s32 mPowerStarId;
    /* 0xC0 */ bool mIsTargetInSight;
    /* 0xC4 */ ActorCameraInfo* mCameraInfo;
    /* 0xC8 */ TVec3f mCameraTarget;
    /* 0xD4 */ ActorCameraInfo* mOpeningCameraInfo;
    /* 0xD8 */ ActorCameraInfo* mDownCameraInfo;
    /* 0xDC */ ActorCameraInfo* mPowerUpCameraInfo;
    /* 0xE0 */ TPos3f mPlayerMtx;
    /* 0x110 */ TPos3f mStartMtx;
};
