#pragma once

#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class YoshiLockOnTarget;

class NeedlePlant : public LiveActor {
public:
    NeedlePlant(const char*);

    virtual ~NeedlePlant();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeWait();
    void exeShake();
    void exeSupportFreeze();
    void exeEatStart();
    void exeEatPull();
    void exeEatPullEnd();
    void exeBroken();
    void breakNeedle();
    bool tryAppearDemoObj();
    bool isNotEating();
    bool isAttackable();
    void updateYoshiLockOn();

    /* 0x90 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0x94 */ LiveActor* mDemoObj;
    /* 0x98 */ TVec3f mTargetPos;
    /* 0xA4 */ s32 mObjArg0;
    /* 0xA8 */ f32 mUnkA8;
    /* 0xAC */ bool mIsFly;
};
