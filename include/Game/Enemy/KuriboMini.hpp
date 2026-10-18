#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class YoshiLockOnTarget;
class WalkerStateWander;
class WalkerStateFindPlayer;
class WalkerStateChase;
class WalkerStateStagger;
class WalkerStateBindStarPointer;

class KuriboMini : public LiveActor {
public:
    KuriboMini(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    bool requestHipDropDown(HitSensor*, HitSensor*);
    bool requestFlatDown(HitSensor*, HitSensor*);
    bool requestPressDown();
    bool requestBlowDown(HitSensor*, HitSensor*);
    bool requestStagger(HitSensor*, HitSensor*);
    bool requestAttackSuccess();
    bool tryFind();
    bool tryPointBind();
    bool tryDeadMap();
    void exeWander();
    void exeFindPlayer();
    void exeChase();
    void exeStagger();
    void exeBindStarPointer();
    void endBindStarPointer();
    void exeAttackSuccess();
    void exeHipDropDown();
    void exeFlatDown();
    void exePressDown();
    void exeBlowDown();
    void calcPassiveMovement();
    bool isEnableAttack() const;
    bool isEnableKick() const NO_INLINE;
    bool isDown() const;

    /* 0x8C */ AnimScaleController* mScaleController;
    /* 0x90 */ WalkerStateWander* mStateWander;
    /* 0x94 */ WalkerStateFindPlayer* mStateFindPlayer;
    /* 0x98 */ WalkerStateChase* mStateChase;
    /* 0x9C */ WalkerStateStagger* mStateStagger;
    /* 0xA0 */ WalkerStateBindStarPointer* mStateBindStarPointer;
    /* 0xA4 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0xA8 */ TQuat4f _A8;
    /* 0xB8 */ TVec3f _B8;
    /* 0xC4 */ bool _C8;
};
