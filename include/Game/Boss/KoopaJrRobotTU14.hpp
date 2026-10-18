#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"

class HitSensor;
class KoopaBattle;
class KoopaBattleCamera;
class KoopaStateBattleStart;
class KoopaStateFireAttack;
class KoopaStateAttackQuake;
class KoopaStateMeteor;
class KoopaStateDamage;

// Final Bowser battle stage controller ("クッパ戦闘段階"). Built by KoopaStateLv1/2/3Sequence (fn_800E28C0).
// It owns the sub-states of one battle phase and runs them through a sequence table (set per stage/variant).
class KoopaBattleStage : public ActorStateBase< KoopaBattle > {
public:
    KoopaBattleStage(KoopaBattle*, s32 stage, s32 variant);  // fn_800E28C0
    virtual ~KoopaBattleStage();                              // fn_800E3400
    virtual void appear();                                    // fn_800E2BF0
    virtual void kill();                                      // fn_800E2C50
    virtual void control();                                   // fn_800E2C90
    virtual void attackSensor(HitSensor*, HitSensor*);        // fn_800E3230
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);  // fn_800E32B0
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);         // fn_800E32C0

    void start(bool);                 // fn_800E2B50
    void exeWait();                   // fn_800E2D40
    void exeDamage();                 // fn_800E2D90
    bool isAttackable() const;        // fn_800E2DF0
    void setSequence(s32);            // fn_800E2E40
    void setSequenceLv1(s32);         // fn_800E2EC0
    void setSequenceLv2(s32);         // fn_800E2F10
    void setSequenceLv3(s32);         // fn_800E2F60
    void changeSub1(s32);             // fn_800E2FE0
    void changeSub2(s32);             // fn_800E3080
    void changeSub3(s32);             // fn_800E3120
    void changeSubs(s32);             // fn_800E2FB0
    bool isInSequence(s32) const;     // fn_800E31F0
    void setNextSequence();           // fn_800E2CA0

    /* 0x10 */ KoopaStateBattleStart* mStateBattleStart;
    /* 0x14 */ KoopaStateFireAttack* mStateAttackFire;
    /* 0x18 */ KoopaStateAttackQuake* mStateAttackQuake;
    /* 0x1C */ KoopaStateMeteor* mStateAttackMeteor;
    /* 0x20 */ KoopaStateDamage* mStateDamage;
    /* 0x24 */ KoopaBattleCamera* mStateExtra;
    /* 0x28 */ const s32* mSequence;
    /* 0x2C */ s32 mSequenceCount;
    /* 0x30 */ s32 mSequenceIndex;
    /* 0x34 */ s32 mUnk34;
    /* 0x38 */ s32 mHp;
    /* 0x3C */ s32 mStage;
    /* 0x40 */ s32 mVariant;
    /* 0x44 */ u32 _44;  // never touched in TU14; allocation size is 0x48 (KoopaStateLv1/2/3Sequence ctors)
};
