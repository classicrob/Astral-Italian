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

    NO_INLINE void exePetoHidden();
    NO_INLINE void exePetoFall();
    NO_INLINE void exePetoLand();
    NO_INLINE void exeExtendsSign();
    NO_INLINE void exeExtends();
    NO_INLINE void exeExtendsKeep();
    void exeShrink();
    NO_INLINE void exeShrinkEnd();
    NO_INLINE void exeJumpSign();
    NO_INLINE void exeJumpArc();
    NO_INLINE void exePetoIdle();
    NO_INLINE void exeSwoonFall();
    NO_INLINE void exeYoshiEaten();
    void exeDown();
    NO_INLINE void exePetoPress();

    NO_INLINE bool isExtending() const;
    NO_INLINE bool isTrampleTarget() const;
    NO_INLINE bool isBindStarEnabled() const;
    NO_INLINE bool startPetoBlowDown(HitSensor*, HitSensor*);
    NO_INLINE bool tryStartPetoPress();
    NO_INLINE void resetToHidden(bool);
    bool updateScale(f32);
    NO_INLINE bool isPetoDamageable() const;
    NO_INLINE void updateDustEffect();
    NO_INLINE void updateJumpTarget();
    NO_INLINE bool isBodySensorActive() const;
    NO_INLINE bool isStarPieceHittable() const;
    NO_INLINE void updateRandomJumpTarget();
    NO_INLINE bool tryFindDustPoint(TVec3f*);

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
    /* 0x130 */ u8 mIsOnceFlag;
    /* 0x131 */ u8 mIsDustEmit;
};
