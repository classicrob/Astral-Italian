#pragma once

#include "Game/Enemy/BrosBase.hpp"
#include "Game/LiveActor/ActorStateBase.hpp"
#include <JSystem/JGeometry/TVec.hpp>

// SMG2 actor states shared by HammerBros / BoomerangBros (unit Game/Enemy/HammerBrosTU02). The host is a BrosBase.
// Original class names are unknown; the symbols are named after their use.

/// Jump along the rail (JumpStart/Jump/JumpEnd). Used by both Bros. sizeof = 0x58.
class BrosStateJump : public ActorStateBase< BrosBase > {
public:
    BrosStateJump(BrosBase*);
    virtual ~BrosStateJump();
    virtual void init();
    virtual void appear();

    void exeJumpStart();    // fn_80189A90
    void exeJump();         // fn_80189F40
    void exeJumpEnd();      // fn_80189FB0
    void selectTarget();    // fn_8018A070
    void updateJump();      // fn_8018A1C0

    /* 0x10 */ s32 mTargetPointNo;
    /* 0x14 */ s32 mCurrentPointNo;
    /* 0x18 */ TVec3f mHomePos;
    /* 0x24 */ TVec3f mStartPos;
    /* 0x30 */ TVec3f mTargetPos;
    /* 0x3C */ TVec3f mFront;
    /* 0x48 */ TVec3f mJumpVelocity;
    /* 0x54 */ s32 mStep;
};

/// HammerBros attack (Attack/AttackSecond/Wait). sizeof = 0x14.
class HammerBrosStateAttack : public ActorStateBase< BrosBase > {
public:
    HammerBrosStateAttack(BrosBase*);
    virtual ~HammerBrosStateAttack();
    virtual void init();
    virtual void appear();

    void exeAttack();        // fn_8018A390
    void exeAttackSecond();  // fn_8018A570
    void exeWait();          // fn_8018A750

    /* 0x10 */ s32 mAttackCount;
};

/// BoomerangBros attack (Attack/AttackSecond/AttackEnd/Catch). sizeof = 0x1C.
class BoomerangBrosStateAttack : public ActorStateBase< BrosBase > {
public:
    BoomerangBrosStateAttack(BrosBase*);
    virtual ~BoomerangBrosStateAttack();
    virtual void init();
    virtual void appear();

    void exeAttack();        // fn_8018A8D0
    void exeAttackSecond();  // fn_8018AAB0
    void exeAttackEnd();     // fn_8018AC90
    void exeCatch();         // fn_8018AD60
    bool tryCatch();         // fn_8018ADF0
    bool isAttackEnd() const;  // fn_8018AE50

    /* 0x10 */ s32 mAttackCount;
    /* 0x14 */ s32 mTimer;
    /* 0x18 */ bool mIsFaceCtrl;
};

/// Swoon (SwoonStart/Swoon/SwoonEnd). Used by both Bros. sizeof = 0x18.
class BrosStateSwoon : public ActorStateBase< BrosBase > {
public:
    BrosStateSwoon(BrosBase*);
    virtual ~BrosStateSwoon();
    virtual void init();
    virtual void appear();
    virtual void control();

    void exeSwoonStart();  // fn_8018B160
    void exeSwoon();       // inlined into the nerve (fn_8018B520)
    void exeSwoonEnd();    // fn_8018B1C0
    void requestBrosSwoonRecovery();
    bool isBrosSwoonVulnerable() const;
    bool isBrosSwoonRecoveryComplete() const;
    bool canBrosSwoonBindStarPointer() const;

    /* 0x10 */ s32 mTimer;
    /* 0x14 */ bool mStartWithRecovery;
};
