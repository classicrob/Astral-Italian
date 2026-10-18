#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class HitSensor;
class MapObjConnector;
class SpinHitController;
class TakoboInkHolder;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;

// SMG2-only unit. Layout from the ctor stores and init/control accesses (offsets are SMG2 real).
class TakoSho : public LiveActor {
public:
    TakoSho(const char*);
    virtual ~TakoSho();

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeMove();
    void exeAttackSign();
    void exeAttack1st();
    void exeAttack2nd();
    void exeRecover();
    void exeSpinHit();
    void exePress();
    void exeHitReaction();
    void exeStun();
    void exeStunEnd();

    NO_INLINE bool isDamaging() const;
    NO_INLINE bool isEnableStun() const;
    NO_INLINE bool isEnableSupportFreeze() const;
    NO_INLINE bool isYoshiEatEnable() const;
    NO_INLINE bool isMoving() const;
    NO_INLINE bool tryStartStun();
    NO_INLINE bool tryStartSupportFreeze();
    NO_INLINE bool tryStartPress(u32);
    NO_INLINE bool tryStartSpinHit(u32, HitSensor*, HitSensor*);
    NO_INLINE void startBrkDamage();
    NO_INLINE void updateFallVelocity();

    /* 0x90 */ TVec3f mFrontVec;
    /* 0x9C */ TVec3f mStartPos;
    /* 0xA8 */ TVec3f mSideVec;
    /* 0xB4 */ bool mIsReverse;
    /* 0xB5 */ bool mIsRight;
    /* 0xB8 */ f32 mMoveRange;
    /* 0xBC */ f32 mSpeed;
    /* 0xC0 */ TVec3f mTargetPos;
    /* 0xCC */ s32 mShotStep;
    /* 0xD0 */ AnimScaleController* mScaleController;
    /* 0xD4 */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0xD8 */ SpinHitController* mSpinHitController;
    /* 0xDC */ TakoboInkHolder* mInkHolder;
    /* 0xE0 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0xE4 */ MapObjConnector* mConnector;
};

// unnamed by address (fn_80208970).
void fn_80208970(TVec3f*, LiveActor*, f32);
