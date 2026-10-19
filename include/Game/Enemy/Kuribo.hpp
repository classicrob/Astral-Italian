#pragma once

class YoshiLockOnTarget;

#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JMapInfo.hpp"
#include <JSystem/JGeometry/TQuat.hpp>

class ItemGenerator;
class WalkerStateStagger;
class WalkerStateChase;
class WalkerStateWander;
class WalkerStateFindPlayer;

class Kuribo : public LiveActor {
public:
    Kuribo(const char*);

    virtual ~Kuribo();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void makeActorDead();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPush(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveMsgEnemyAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void initState();
    void initAppearState(const JMapInfoIter&);
    bool requestDead();
    bool requestFlatDown(u32, HitSensor*, HitSensor*);
    bool requestHipDropDown(u32, HitSensor*, HitSensor*);
    bool requestPressDown();
    bool requestStagger(HitSensor*, HitSensor*);
    bool requestBlowDown(u32, HitSensor*, HitSensor*);
    bool requestAttackSuccess();
    void onNoGravitySupport();
    void setUp(const TVec3f&, const TQuat4f&, const TVec3f&);
    void appearBlowed(const TVec3f&, const TQuat4f&, const TVec3f&);
    void appearHipDropped(const TVec3f&, const TQuat4f&);
    void appearKuriboWandering(const TVec3f&, const TQuat4f&, const TVec3f&);
    bool requestKuriboStaggerWithoutRecovery(HitSensor*, HitSensor*);
    void appearKuriboFromBox(const TVec3f&, const TQuat4f&);
    void appearKuriboHipDroppedWithoutBind(const TVec3f&, const TQuat4f&);
    void exeKuriboLaunch();
    bool canKuriboStartStagger() const;
    bool canKuriboReceiveStagger() const;
    bool canKuriboBeEatenByYoshi() const;
    bool tryNonActive();
    bool tryActive();
    bool tryEndBlow();
    bool tryEndBlowLand();
    bool tryFind();
    bool tryPointBind();
    bool tryDead();
    void exeWander();
    void exeFindPlayer();
    void exeChase();
    void exeStagger();
    void exeNonActive();
    void endNonActive();
    void exeAppearFromBox();
    void exeBlow();
    void exeAttackSuccess();
    void exeHipDropDown();
    void exeFlatDown();
    void exePressDown();
    void exeBlowDown();
    void exeBindStarPointer();
    void exeBlowLand();
    void calcPassiveMovement();
    bool isEnableKick() const;
    bool isUpsideDown() const;
    bool isEnableDead() const;
    bool isEnablePointBind() const;
    bool isEnableAttack() const;
    bool isEnableStamp() const;
    bool isEnablePanch() const;
    bool isEnablePushMove() const;

    /* 0x90 */ AnimScaleController* mScaleController;
    /* 0x94 */ WalkerStateWander* mStateWander;
    /* 0x98 */ WalkerStateFindPlayer* mStateFindPlayer;
    /* 0x9C */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0xA0 */ WalkerStateStagger* mStateStagger;
    /* 0xA4 */ WalkerStateChase* mStateChase;
    /* 0xA8 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0xAC */ TQuat4f mOrientation;
    /* 0xBC */ TVec3f mFrontDirection;
    /* 0xC8 */ s32 mAppearState;
    /* 0xCC */ bool mNoGravitySupport;
    /* 0xCD */ u8 mItemDropsEnabled;
    /* 0xCE */ bool mWasUpsideDown;
};

namespace MR {
    LiveActor* createNoItemKuriboActor(const char*);
};  // namespace MR
