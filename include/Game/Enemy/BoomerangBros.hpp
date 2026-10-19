#pragma once

#include "Game/Enemy/BrosBase.hpp"
#include "Game/Enemy/BrosProjectileBase.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class WalkerStateBindStarPointer;
class BrosStateJump;
class BoomerangBrosStateAttack;
class BrosStateSwoon;

namespace MR {
    bool isMsgYoshiReleaseOrCancelEaten(u32);
    bool isMsgYoshiStartTongue(u32);
    bool isMsgYoshiLoopTongue(u32);
    bool isMsgEnemyAttack(u32);
};  // namespace MR

class BoomerangBros : public BrosBase {
public:
    BoomerangBros(const char*);

    virtual ~BoomerangBros();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual const TVec3f& getHoldTrans() const;   // fn_80171F90
    virtual const TVec3f& getHoldRotate() const;  // fn_80171FA0
    virtual void throwProjectile(BrosProjectileBase*);  // fn_801736B0

    void initStates();  // fn_80171D10
    void exeWait();                  // fn_80171FB0
    void exeInterval();              // fn_80172090
    void exeJump();                  // fn_80172100
    void exeAttack();                // fn_80172160
    void exeSwoon();                 // fn_801721B0
    void exeBlowDown();              // fn_80172230
    void exeSwoonPressDown();        // fn_80172320
    void exeStandPressDown();        // fn_801723B0
    void exeGuard();                 // fn_801724E0
    void exeEaten();
    void updateVelocity2();          // fn_80172BD0
    bool isVulnerable() const;       // fn_80172C60
    bool isNormalNerve() const;      // fn_80172CE0
    bool isGuardable() const;        // fn_80172D90
    bool isCatchNerve() const;       // fn_80172E10
    bool isAliveNerve() const;       // fn_80172EC0
    bool isStarPointerBindable() const;  // fn_80172F70
    bool tryExplosionDamage(HitSensor*, HitSensor*);  // fn_80173050
    bool tryKickDamage(HitSensor*, HitSensor*);       // fn_801730D0
    bool tryHitDamage(HitSensor*, HitSensor*);        // fn_801731A0
    bool trySpinDamage(HitSensor*, HitSensor*);       // fn_80173230
    bool tryStomp();                                  // fn_80173350
    bool tryStarPieceReflect();                       // fn_80173480
    bool tryYoshiTongue(HitSensor*, HitSensor*);      // fn_801734E0
    bool tryBindStarPointer();                        // fn_801735D0

    /* 0x0D8 */ WalkerStateBindStarPointer* mBindStand;
    /* 0x0DC */ WalkerStateBindStarPointer* mBindSwoon;
    /* 0x0E0 */ f32 mUnusedE0;
    /* 0x0E4 */ s32 mWaitCount;
    /* 0x0E8 */ BrosStateJump* mStateJump;
    /* 0x0EC */ BoomerangBrosStateAttack* mStateAttack;
    /* 0x0F0 */ BrosStateSwoon* mStateSwoon;
};

/// @brief The boomerang thrown by BoomerangBros (SMG2). Only the simple methods are ported.
class BoomerangBrosBoomerang : public BrosProjectileBase {
public:
    BoomerangBrosBoomerang(const char*);

    virtual ~BoomerangBrosBoomerang();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual void v70(const TVec3f&, const TVec3f&);   // fn_80173EF0
    virtual void attachToBros(BrosBase*);             // fn_80173CE0
    virtual bool v78();                               // fn_80174890
    virtual bool v7C();                               // fn_801748A0
    virtual bool v80();                               // fn_801748B0
    virtual bool v84();                               // fn_801748C0
    virtual bool v88();                               // fn_80174920
    virtual bool v8C();                               // fn_801749C0
    virtual bool v90();                               // fn_80174A30

    void exeFly();         // fn_80173FE0
    void exeOut();         // fn_80174060
    void exeBack();        // fn_80174240
    void exeTongue();      // fn_801742D0
    void exeThrown();      // fn_80174330
    void exeThrownBack();  // fn_80174560
    void calcPath(TVec3f*, f32);  // fn_80174650: point of the out/back flight curve at t
    bool isReturning() const;     // fn_801747E0

    /* 0xBC */ f32 mPathDistance;
    /* 0xC0 */ s32 mStep;
    /* 0xC4 */ TPos3f mMtx;
    /* 0xF4 */ WalkerStateBindStarPointer* mBind;
};
