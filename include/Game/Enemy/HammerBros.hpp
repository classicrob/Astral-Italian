#pragma once

#include "Game/Enemy/BrosBase.hpp"
#include "Game/Enemy/BrosProjectileBase.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class WalkerStateBindStarPointer;
class HammerBrosStateJump;
class HammerBrosStateAttack;
class HammerBrosStateSwoon;

namespace MR {
    bool isMsgYoshiReleaseOrCancelEaten(u32);
    bool isMsgYoshiStartTongue(u32);
    bool isMsgYoshiLoopTongue(u32);
};  // namespace MR

class HammerBros : public BrosBase {
public:
    HammerBros(const char*);

    virtual ~HammerBros();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual const TVec3f& getHoldTrans() const;
    virtual const TVec3f& getHoldRotate() const;
    virtual void throwProjectile(BrosProjectileBase*);

    void initStates();                                          // fn_80186DC0
    void exeWait();                                             // fn_80187050
    void exeInterval();                                         // fn_80187140
    void exeJump();                                             // fn_801871A0
    void exeAttack();                                           // fn_80187200
    void exeSwoon();                                            // fn_80187250
    void exeBlowDown();                                         // fn_801872D0
    void exeSwoonPressDown();                                   // fn_801873C0
    void exeStandPressDown();                                   // fn_80187460
    void exeGuard();                                            // fn_80187590
    void exeEaten();
    bool isVulnerable() const;                                  // fn_80187B80
    bool isNormalNerve() const;                                 // fn_80187C00
    bool isGuardable() const;                                   // fn_80187CB0
    bool isAliveNerve() const;                                  // fn_80187D30
    bool isStarPointerBindable() const;                         // fn_80187E10
    bool tryExplosionDamage(HitSensor*, HitSensor*);            // fn_80187EF0
    bool tryKickDamage(HitSensor*, HitSensor*);                 // fn_80187F70
    bool tryHitDamage(HitSensor*, HitSensor*);                  // fn_80188040
    bool trySpinDamage(HitSensor*, HitSensor*);                 // fn_801880D0
    bool tryStomp();                                            // fn_801881F0
    bool tryStarPieceReflect();                                 // fn_80188320
    bool tryYoshiTongue(HitSensor*, HitSensor*);                // fn_80188380
    bool tryBindStarPointer();                                  // fn_80188470

    /* 0x0D8 */ WalkerStateBindStarPointer* mBindStand;
    /* 0x0DC */ WalkerStateBindStarPointer* mBindSwoon;
    /* 0x0E0 */ f32 _E0;
    /* 0x0E4 */ s32 mWaitCount;
    /* 0x0E8 */ TPos3f mMtx;
    /* 0x118 */ HammerBrosStateJump* mStateJump;
    /* 0x11C */ HammerBrosStateAttack* mStateAttack;
    /* 0x120 */ HammerBrosStateSwoon* mStateSwoon;
};

class HammerBrosHammer : public BrosProjectileBase {
public:
    HammerBrosHammer(const char*);

    virtual ~HammerBrosHammer();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void v70(const TVec3f&, const TVec3f&);  // fn_80188B40: launch
    virtual bool v78();  // fn_801894B0: isNerve(Fly)
    virtual bool v7C();  // fn_801894C0: isNerve(Tongue)
    virtual bool v80();  // fn_801894D0: isNerve(Eaten)
    virtual bool v84();  // fn_801894E0: isNerve(Thrown)
    virtual bool v88();                               // fn_801894F0
    virtual bool v8C();                               // fn_801895C0
    virtual bool v90();                               // fn_80189630

    void exeFly();     // fn_801890E0
    void exeSpin();    // fn_80189160
    void exeTongue();  // fn_801892D0
    void exeThrown();  // fn_80189330

    /* 0xBC */ WalkerStateBindStarPointer* mBind;
};
