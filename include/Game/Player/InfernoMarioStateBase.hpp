#pragma once
#include "Game/LiveActor/ActorStateBase.hpp"
class InfernoMario;
class HitSensor;
class InfernoMarioStateBase : public ActorStateBase<InfernoMario> {
public:
    InfernoMarioStateBase(const char*, InfernoMario*);
    virtual ~InfernoMarioStateBase();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual bool start();
    virtual bool startFrame(s32);
    virtual void fn_80381750() {}
    virtual void fn_80381780() {}
};
class InfernoMarioComet : public InfernoMarioStateBase {
public:
    InfernoMarioComet(InfernoMario*);
    virtual ~InfernoMarioComet();
    virtual void appear();
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual bool start();
    virtual bool startFrame(s32);
    bool isFollowing() const NO_INLINE;
    void exeWait();
    void exeFollow();
    void exeAppearance();
    s32 mFrame;
};
