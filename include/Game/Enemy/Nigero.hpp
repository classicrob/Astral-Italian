#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TQuat.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include "Game/LiveActor/HitSensor.hpp"

class AnimScaleController;
class WalkerStateBindStarPointer;
class ParabolicPath;
class YoshiLockOnTarget;
class HitSensor;

/// @brief Nigero (a small enemy that walks toward the player, hides and comes back).
/// Layout from the SB4E01 ctor (LiveActor is 0x90 bytes in SMG2).
class Nigero : public LiveActor {
public:
    Nigero(const char*);
    virtual ~Nigero();

    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeAppear();
    void exeWaitNormal();
    void exeWalk();
    void exeWalkMove();
    void exeSearch();
    void exeFind();
    void exeChase();
    void exeFlee();
    void exeSwoon();
    void exeGet();
    void exeDead();
    void exeHide();
    void exeHidden();
    void exeAttackSuccess();
    void exeSupportFreeze();

    bool recvEyeAttack(HitSensor*, HitSensor*);
    bool recvItemGet(HitSensor*, HitSensor*);
    bool recvBodyNerve(HitSensor*, HitSensor*);
    bool recvAttackSensor(HitSensor*, HitSensor*);
    bool recvPushDefault(HitSensor*, HitSensor*);
    bool isBindedNerve() const;  // fn_801D1850
    bool isEatenNerve() const;   // fn_801D1910
    bool isLockOnNerve() const;  // fn_801D19B0
    bool isActiveNerve() const;  // fn_801D1A50
    bool isWalkNerve() const;    // fn_801D1B20
    void recvLostTarget();       // fn_801D1B90
    void recvStarPieceEnd();     // fn_801D1C20
    void moveToTarget(const TVec3f&, f32, f32, f32, f32, f32);  // fn_801D2840
    void updateUpVec();                                           // fn_801D2C30

    /* 0x90 */ TQuat4f mRotate;
    /* 0xA0 */ TQuat4f mRotateTarget;
    /* 0xB0 */ HitSensor* mHoldSensor;
    /* 0xB4 */ HitSensor* mTargetSensor;
    /* 0xB8 */ ParabolicPath* mPath;
    /* 0xBC */ AnimScaleController* mAnimScaleCtrl;
    /* 0xC0 */ WalkerStateBindStarPointer* mBindStar;
    /* 0xC4 */ TVec3f mMoveDir;
    /* 0xD0 */ TVec3f mFront;
    /* 0xDC */ TVec3f mResetPos;
    /* 0xE8 */ s32 mUpCounter;
    /* 0xEC */ s32 mAwayTimer;
    /* 0xF0 */ YoshiLockOnTarget* mYoshiLockOn;
};
