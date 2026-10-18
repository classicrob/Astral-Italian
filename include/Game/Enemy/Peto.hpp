#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
#include <JSystem/JGeometry/TQuat.hpp>
#include <JSystem/JGeometry/TVec.hpp>

class AnimScaleController;
class WalkerStateBindStarPointer;
class VolumeDrawer;
class YoshiLockOnTarget;
class ActorStateParamScale;
class HitSensor;

/// @brief Peto (a balloon-like enemy that extends/shrinks and jumps at the player).
/// Layout from the SB4E01 ctor (LiveActor is 0x90 bytes in SMG2).
class Peto : public LiveActor {
public:
    Peto(const char*);
    virtual ~Peto();

    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void movement();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeHidden();
    void exeFall();
    void exeLand();
    void exeExtendsSign();
    void exeExtends();
    void exeExtendsKeep();
    void exeShrink();
    void exeShrinkEnd();
    void exeJumpSign();
    void exeJump();
    void exeWait();
    void exeSwoon();
    void exeYoshiEaten();
    void exeDown();
    void exePress();

    bool isHitNerve() const;
    bool isLandNerve() const;
    bool isPressNerve() const;
    bool recvDamage(HitSensor*, HitSensor*);
    bool recvTrample();
    bool recvExplosion();
    void resetState(bool);
    bool updateScale(f32);
    bool recvDamageCheck() const;
    void updateJumpVel();
    void updateOnRail();
    bool isNearPlayerSensor();
    void spawnDust();

    /* 0x90 */ AnimScaleController* mAnimScale;
    /* 0x94 */ WalkerStateBindStarPointer* mBindStar;
    /* 0x98 */ VolumeDrawer* mVolumeDrawer;
    /* 0x9C */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0xA0 */ ActorStateParamScale* mStateParam;
    /* 0xA4 */ TMtx34f mMtx;
    /* 0xD4 */ TQuat4f mQuat;
    /* 0xE4 */ TVec3f mFront;
    /* 0xF0 */ TVec3f mJumpPos;
    /* 0xFC */ TVec3f mJumpStart;
    /* 0x108 */ f32 mScale;
    /* 0x10C */ TVec3f mRailPos;
    /* 0x118 */ TVec3f mWalkPos;
    /* 0x124 */ TVec3f mDustPos;
    /* 0x130 */ s8 mIsOnceFlag;
    /* 0x131 */ s8 mIsDustEmit;
};
