#pragma once

#include "Game/Boss/KoopaStateBase.hpp"
#include <JSystem/JGeometry.hpp>

class JMapInfoIter;
class LiveActor;
class LiveActorGroup;
class KoopaBattle;

// Battle phase states created by the Lv sequences (units KoopaJrRobotTU14 / TU15, not part of the Koopa state family units).
class KoopaStateBattle : public KoopaStateBase {
public:
    KoopaStateBattle(KoopaBattle*, s32, s32);  // fn_800E28C0

    virtual ~KoopaStateBattle();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void start(bool);  // fn_800E2B50

    u8 _10[0x38];
};

class KoopaStateLastBattle : public KoopaStateBase {
public:
    KoopaStateLastBattle(KoopaBattle*);  // fn_800E3630

    virtual ~KoopaStateLastBattle();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    u8 _10[0x18];
};

class KoopaStateOpeningDemo;
class KoopaStatePowerUpDemo;
class KoopaStateDownDemo;
class KoopaStatePreFinalDemo;
class KoopaStateEndDemo;

// Phase sequences of the final Bowser battle (created by KoopaBattle::init).
class KoopaStateLv1Sequence : public KoopaStateBase {
public:
    KoopaStateLv1Sequence(KoopaBattle*, const JMapInfoIter&);

    virtual ~KoopaStateLv1Sequence();
    virtual void appear();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeDemoOpening();
    void exeBattleLv1();
    void exeDemoPowerUp();
    void exeBattleLv2();
    void exeDemoDown();

    /* 0x10 */ KoopaStateBattle* mBattleLv1;
    /* 0x14 */ KoopaStateBattle* mBattleLv2;
    /* 0x18 */ KoopaStatePowerUpDemo* mPowerUpDemo;
    /* 0x1C */ LiveActor* mSky;
};

class KoopaStateLv2Sequence : public KoopaStateBase {
public:
    KoopaStateLv2Sequence(KoopaBattle*, const JMapInfoIter&);

    virtual ~KoopaStateLv2Sequence();
    virtual void appear();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeDemoOpening();
    void exeBattleLv1();
    void exeDemoPowerUp();
    void exeBattleLv2();
    void exeDemoDown();

    /* 0x10 */ KoopaStateBattle* mBattleLv1;
    /* 0x14 */ KoopaStateBattle* mBattleLv2;
    /* 0x18 */ KoopaStatePowerUpDemo* mPowerUpDemo;
    /* 0x1C */ KoopaStateDownDemo* mDownDemo;
    /* 0x20 */ LiveActor* mSky;
};

class KoopaStateLv3Sequence : public KoopaStateBase {
public:
    KoopaStateLv3Sequence(KoopaBattle*, const JMapInfoIter&);

    virtual ~KoopaStateLv3Sequence();
    virtual void appear();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeDemoOpening();
    void exeBattleLv1();
    void exeDemoPowerUp();
    void exeBattleLv2();
    void exeLastBattleStart();
    void exeLastBattle();
    void exeDemoLastBattleEnd();

    /* 0x10 */ KoopaStateBattle* mBattleLv1;
    /* 0x14 */ KoopaStateBattle* mBattleLv2;
    /* 0x18 */ KoopaStatePowerUpDemo* mPowerUpDemo;
    /* 0x1C */ KoopaStateLastBattle* mLastBattle;
    /* 0x20 */ LiveActor* mSky;
    /* 0x24 */ LiveActor* mLastBattleSky;
};

// TU08: demo played when the final Bowser powers up.
class KoopaStatePowerUpDemo : public ActorStateBase< KoopaBattle > {
public:
    KoopaStatePowerUpDemo(KoopaBattle*);

    virtual ~KoopaStatePowerUpDemo();
    virtual void appear();

    void exeWait() {}
    void exeDemo();

    /* 0x10 */ bool mIsCameraInterpolating;
};

// TU02: battle start (Bowser descends into the arena).
class KoopaStateBattleStart : public ActorStateBase< KoopaBattle > {
public:
    KoopaStateBattleStart(KoopaBattle*);

    virtual ~KoopaStateBattleStart();
    virtual void appear();

    void initPosition();
    void exeBattleStartLoop();
    void exeBattleStartEnd();

    /* 0x10 */ TVec3f mStartPos;
    /* 0x1C */ TVec3f mEndPos;
};

// TU03: damage reaction (animation depends on which body part was hit).
class KoopaStateDamage : public ActorStateBase< KoopaBattle > {
public:
    KoopaStateDamage(KoopaBattle*, s32);

    virtual ~KoopaStateDamage();
    virtual void appear();

    void setDamageType(HitSensor*);
    void setFinal(bool);
    void exeDamage();
    void exeDamageEnd();

    /* 0x10 */ s32 mDamageType;
    /* 0x14 */ s32 mParam;
    /* 0x18 */ bool mIsFinal;
};

// TU05: demo after the final hit (Bowser falls down, Power Star appears).
class KoopaStateDownDemo : public ActorStateBase< KoopaBattle > {
public:
    KoopaStateDownDemo(KoopaBattle*, const JMapInfoIter&, const char*, const char*);

    virtual ~KoopaStateDownDemo();
    virtual void appear();
    virtual void kill();

    void exeWait();
    void exeDemo();

    /* 0x10 */ LiveActor* mTicoBaby;
    /* 0x14 */ LiveActor* mPowerStarModel;
};

// TU07: opening demo of the final Bowser battle (Peach in the bubble cage is optional).
class KoopaStateOpeningDemo : public ActorStateBase< KoopaBattle > {
public:
    KoopaStateOpeningDemo(KoopaBattle*, const JMapInfoIter&, const char*, const char*, bool);

    virtual ~KoopaStateOpeningDemo();
    virtual void appear();
    virtual void kill();

    void exeWait();
    void exeDemo();

    /* 0x10 */ LiveActor* mTicoBaby;
    /* 0x14 */ LiveActor* mPeach;
    /* 0x18 */ LiveActor* mPeachCage;
};

// TU04: defense form (Bowser withdraws into his shell).
class KoopaStateDefense : public ActorStateBase< KoopaBattle > {
public:
    KoopaStateDefense(KoopaBattle*, bool);

    virtual ~KoopaStateDefense();
    virtual void appear();

    void setStartActionName(const char*);
    void exeStart();
    void exeWait();
    void exeClose();
    void exeEnd();

    /* 0x10 */ const char* mStartActionName;
    /* 0x14 */ const char* mEndActionName;
    /* 0x18 */ bool mFlag;
};

// TU06: demo after the last battle (Mario, Peach and the Grand Star).
class KoopaStateEndDemo : public ActorStateBase< KoopaBattle > {
public:
    KoopaStateEndDemo(KoopaBattle*);

    virtual ~KoopaStateEndDemo();
    virtual void appear();
    virtual void kill();

    void setCastActor(LiveActor*);
    void initDemo(const JMapInfoIter&);
    void lightOnPeach();
    void lightOnLastStar();
    void exeWait();
    void exeDemo();

    /* 0x10 */ LiveActor* mPeach;
    /* 0x14 */ LiveActor* mPowerStarModel;
    /* 0x18 */ LiveActor* mCastActor;
    /* 0x1C */ LiveActor* mTicoBaby;
};

// TU09: fire attack (spits fire balls while turning around the arena).
class KoopaStateFireAttack : public ActorStateBase< KoopaBattle > {
public:
    KoopaStateFireAttack(KoopaBattle*);

    virtual ~KoopaStateFireAttack();
    virtual void appear();
    virtual void kill();

    void setFireCount(s32);
    void moveAroundArena(f32);
    void exeFireStart();
    void exeFireLoop();
    void exeFireEnd();

    /* 0x10 */ LiveActorGroup* mFireBalls;
    /* 0x14 */ TVec3f _14;
    /* 0x20 */ s32 mFireCount;
    /* 0x24 */ bool mIsLoopStarted;
};

// TU10: last attack (rolls around the arena, then charges the player).
class KoopaStateLastAttack : public ActorStateBase< KoopaBattle > {
public:
    KoopaStateLastAttack(KoopaBattle*);

    virtual ~KoopaStateLastAttack();
    virtual void appear();
    virtual void kill();
    virtual void attackSensor(HitSensor*, HitSensor*);

    bool isAttacking() const;
    void exeChaseRoll();
    void exeAttackSign();
    void exeMoveLoop();
    void exeAttack();
    void exeAttackEnd();
    void exeMoveEndRoll();
    void exeMoveEndLeft();

    /* 0x10 */ TQuat4f mQuatA;
    /* 0x20 */ TQuat4f mQuatB;
    /* 0x30 */ TVec3f mStartPos;
    /* 0x3C */ TVec3f mTargetPos;
    /* 0x48 */ s32 mTargetIdx;
    /* 0x4C */ bool mIsChaseRoll;
};

// TU11: sequence before the last battle (Tico Baby, Power Star demo, "Grand Star" light switch).
class KoopaStatePreFinalDemo : public ActorStateBase< KoopaBattle > {
public:
    KoopaStatePreFinalDemo(KoopaBattle*, const JMapInfoIter&);

    virtual ~KoopaStatePreFinalDemo();
    virtual void appear();
    virtual void kill();
    virtual void initDemo(const JMapInfoIter&);

    void setCastActor(LiveActor*);
    void lightOnFinalBattle();
    void exeWait();
    void exeDemoDown();
    void exeStayLoop();
    void exeDemoStart();
    void exeDemoEnd();

    /* 0x10 */ LiveActor* mTicoBaby;
    /* 0x14 */ LiveActor* _14;
    /* 0x18 */ LiveActor* mPowerStarModel;
    /* 0x1C */ LiveActor* mCastActor;
};

// TU13: damage of the last battle (Bowser tumbles away, or is knocked down for the final blow).
class KoopaStateLastDamage : public ActorStateBase< KoopaBattle > {
public:
    KoopaStateLastDamage(KoopaBattle*);

    virtual ~KoopaStateLastDamage();
    virtual void appear();

    void moveToTarget();
    void setDamageType(HitSensor*);
    void exeDamage();
    void exeRollEnd();

    /* 0x10 */ TQuat4f mQuatA;
    /* 0x20 */ TVec3f mStartPos;
    /* 0x2C */ TVec3f mTargetPos;
    /* 0x38 */ s32 mStep;
    /* 0x3C */ s32 mDamageType;
    /* 0x40 */ s32 mDuration;
    /* 0x44 */ bool mIsLongDamage;
    /* 0x45 */ bool mIsFinal;
};
