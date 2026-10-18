#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class DemoObj;
class SpringValue;

class KoopaJrCastleCloudStep : public LiveActor {
public:
    KoopaJrCastleCloudStep(const char*);

    virtual ~KoopaJrCastleCloudStep();
    virtual void init(const JMapInfoIter&) override;
    virtual void makeActorAppeared() override;
    virtual void makeActorDead() override;
    virtual void control() override;
    virtual void calcAndSetBaseMtx() override;
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) override;
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*) override;
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*) override;

    bool tryDie();
    bool updateWind();
    void updateBrkByCamera();
    void exeWait();
    void exeBounce();
    void exeBorn();
    void exeDie();

    /* 0x90 */ DemoObj* mDemoObj;
    /* 0x94 */ SpringValue* mSpring;
    /* 0x98 */ TVec3f mHomePos;
    /* 0xA4 */ s32 mBounceTimer;
    /* 0xA8 */ s32 mElecTimer;
    /* 0xAC */ f32 mBrkFrame;
    /* 0xB0 */ bool mUseShadow;
};
