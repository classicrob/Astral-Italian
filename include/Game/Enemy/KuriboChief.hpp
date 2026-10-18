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
    bool fn_801BC910() const;
    void fn_801BC140();
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
    // PLACEHOLDER(enemies): fn_801BBFB0 and fn_801BC000 (unnamed KuriboChief members at 0x801BBFB0/0x801BC000, used by PatakuriBig); resolve when the KuriboChief unit names them.
    void fn_801BBFB0(const TVec3f&, const TQuat4f&, const TVec3f&);
    void fn_801BC000(const TVec3f&, const TQuat4f&, const TVec3f&, HitSensor*, HitSensor*);

    /* 0x8C */ AnimScaleController* mScaleController;
    /* 0x90 */ WalkerStateWander* mStateWander;
    /* 0x94 */ WalkerStateFindPlayer* mStateFindPlayer;
    /* 0x98 */ WalkerStateChase* mStateChase;
    /* 0x9C */ WalkerStateStagger* mStateStagger;
    /* 0xA0 */ WalkerStateBindStarPointer* mStateBindStarPointer;
    /* 0xA8 */ KeySwitch* mKeySwitch;
    /* 0xAC */ TQuat4f _AC;
    /* 0xBC */ TVec3f _BC;
    /* 0xC8 */ bool _C8;
};
