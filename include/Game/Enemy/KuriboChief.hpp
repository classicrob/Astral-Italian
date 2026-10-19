#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/NameObj/NameObjArchiveListCollector.hpp"

class AnimScaleController;
class WalkerStateWander;
class WalkerStateFindPlayer;
class WalkerStateChase;
class WalkerStateStagger;
class WalkerStateBindStarPointer;
class KeySwitch;

class KuriboChief : public LiveActor {
public:
    bool isBlowDownNerve() const;
    bool tryDieInHazard();
    KuriboChief(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void initState();
    void initKeySwitch(const JMapInfoIter&);
    static void makeArchiveList(NameObjArchiveListCollector*, const JMapInfoIter&);
    bool requestStagger(HitSensor*, HitSensor*);
    bool requestBlowDown(HitSensor*, HitSensor*);
    bool tryFind();
    bool tryPointBind();
    void exeWander();
    void exeFindPlayer();
    void exeChase();
    void exeStagger();
    void exeTrample();
    void exeBindStarPointer();
    void exeAttackSuccess();
    void exeBlowDown();
    void exeBlowDownLand();
    bool isEnableAttack() const;
    bool isEnableKick() const;
    bool isDown() const;
    void setPositionRotationVelocity(const TVec3f&, const TQuat4f&, const TVec3f&);
    void appearAndWander(const TVec3f&, const TQuat4f&, const TVec3f&);
    void appearAndStagger(const TVec3f&, const TQuat4f&, const TVec3f&, HitSensor*, HitSensor*);

    /* 0x90 */ AnimScaleController* mScaleController;
    /* 0x94 */ WalkerStateWander* mStateWander;
    /* 0x98 */ WalkerStateFindPlayer* mStateFindPlayer;
    /* 0x9C */ WalkerStateChase* mStateChase;
    /* 0xA0 */ WalkerStateStagger* mStateStagger;
    /* 0xA4 */ WalkerStateBindStarPointer* mStateBindStarPointer;
    /* 0xA8 */ KeySwitch* mKeySwitch;
    /* 0xAC */ TQuat4f _AC;
    /* 0xBC */ TVec3f _BC;
    /* 0xC8 */ bool _C8;
};
