#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class HitSensor;
class MapObjConnector;
class SpinHitController;
class TakoboInkHolder;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;

// SMG2 Takobo (the blue octopus; sibling of TakoSho, same layout). Offsets from the ctor stores.
class Takobo : public LiveActor {
public:
    Takobo(const char*);
    virtual ~Takobo();

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeIdle();  // fn_80203EC0 (Wait / WaitAfter nerves)
    void exeMove();  // fn_80203FB0
    void exeAttackSign();  // fn_802041D0
    void exeAttack();      // fn_80204270
    void exeWait();        // Recover nerve
    void exeHitPunch();
    void exePress();
    void exeHitReaction();
    void exeStun();        // fn_802045C0
    void exeStunEnd();

    NO_INLINE bool tryStartSpinHit(u32, HitSensor*, HitSensor*);  // fn_802046F0
    NO_INLINE bool tryStartPress(u32);                            // fn_802047A0
    NO_INLINE bool tryStartStun();                                // fn_80204830
    NO_INLINE bool tryStartSupportFreeze();                       // fn_80204890
    NO_INLINE bool isYoshiEatEnable() const;                      // fn_80204900
    NO_INLINE bool isEnableStun() const;                          // fn_80204970
    NO_INLINE bool isEnableSupportFreeze() const;                 // fn_802049E0
    NO_INLINE bool isDamaging() const;                            // fn_80204A90
    NO_INLINE void startBrkDamage();                              // fn_80204B00
    NO_INLINE bool isMoving() const;                              // fn_80204B70
    NO_INLINE bool isTakoboPlayerInAttackRange() const;
    NO_INLINE void updateFallVelocity();                          // fn_80204C20

    /* 0x90 */ TVec3f mFrontVec;
    /* 0x9C */ TVec3f mStartPos;
    /* 0xA8 */ TVec3f mSideVec;
    /* 0xB4 */ bool mIsReverse;
    /* 0xB5 */ bool mIsRight;
    /* 0xB8 */ f32 mMoveRange;
    /* 0xBC */ f32 mSpeed;
    /* 0xC0 */ TVec3f mTargetPos;
    /* 0xCC */ s32 mMoveStep;
    /* 0xD0 */ AnimScaleController* mScaleController;
    /* 0xD4 */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0xD8 */ SpinHitController* mSpinHitController;
    /* 0xDC */ TakoboInkHolder* mInkHolder;
    /* 0xE0 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0xE4 */ MapObjConnector* mConnector;
};
