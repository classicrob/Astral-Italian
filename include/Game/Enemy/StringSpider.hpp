#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class SpinHitController;
class AnimScaleController;
class WalkerStateBindStarPointer;
class YoshiLockOnTarget;
class StringSpiderString;
class FixedPosition;

class StringSpider : public LiveActor {
public:
    StringSpider(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void calcAnim();
    virtual void kill();
    virtual void makeActorDead();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void doSwingAction(f32);
    void fn_80201450();
    void reactStringForce();
    bool isOverStringLength(TVec3f*, f32*);
    TVec3f calcToRootVector() NO_INLINE;
    void calcPosture(f32, bool);
    void extendString();
    void shortenString(f32);
    void startSwingLevelSound();
    bool fn_80202B40() const;
    bool isEatable() const;
    bool isDamaging() const;
    bool fn_80202D00() const;
    bool fn_80202D80() const;
    bool fn_80202DE0() const;

    void exeWait();
    void endWait();
    void exeDownStart();
    void exeDownEnd();
    void exeDown();
    void exeUp();
    void exeSearch();
    void exeAttack();
    void exeHit();
    void exeSpinned();
    void exeBlow();

    /* 0x90 */ SpinHitController* mSpinHitController;
    /* 0x94 */ AnimScaleController* mScaleController;
    /* 0x98 */ StringSpiderString* mString;
    /* 0x9C */ WalkerStateBindStarPointer* mBindState;
    /* 0xA0 */ FixedPosition* mFixedPos;
    /* 0xA4 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0xA8 */ TQuat4f mPoseQuat;
    /* 0xB8 */ TQuat4f mInitPoseQuat;
    /* 0xC8 */ f32 mMaxLength;
    /* 0xCC */ f32 mCurrentLength;
};

class StringSpiderString : public LiveActor {
public:
    StringSpiderString(const char*);

    virtual void calcAndSetBaseMtx();

    /* 0x90 */ TQuat4f mRotation;
    /* 0xA0 */ TVec3f mEndPos;
};
