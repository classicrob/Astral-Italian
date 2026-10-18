#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class WalkerStateBindStarPointer;
class ScatterFireBallLava;

class ScatterFireBall : public LiveActor {
public:
    ScatterFireBall(const char* = "拡散ファイアーボール");
    virtual ~ScatterFireBall();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    HitSensor* fn_8017EBB0() const;
    bool fn_8017EC30();
    void fn_8017ED40(f32);
    void fn_8017EDF0();
    void fn_8017EE80();
    void fn_8017EF90();
    void exeFreeze();

    void fn_8017E490(const TVec3f&, f32, f32);
    void fn_8017E660(const TVec3f&, const TVec3f&);
    void fn_8017E6D0(const TVec3f&, const TVec3f&);
    bool fn_8017E750();
    bool fn_8017E7B0();

    /* 0x90 */ LiveActor* mHost;
    /* 0x94 */ AnimScaleController* _94;
    /* 0x98 */ WalkerStateBindStarPointer* _98;
    /* 0x9C */ TVec3f _9C;
    /* 0xA8 */ TVec3f _A8;
    /* 0xB4 */ f32 _B4;
    /* 0xB8 */ ScatterFireBallLava* _B8;
    /* 0xBC */ s32 _BC;
    /* 0xC0 */ bool _C0;
};
