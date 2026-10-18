#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class YoshiLockOnTarget;
class AnimScaleController;
class WalkerStateBindStarPointer;

namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, MtxPtr, const TVec3f&);
};

class Terebo : public LiveActor {
public:
    Terebo(const char*);
    virtual ~Terebo();

    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor*, HitSensor*);
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);

    void exeAppear();
    void exeWait();
    void exeHide();
    void exeHitReaction();
    void exeSpinReaction();
    void exeTongue();
    void exeDeath();
    void exeSupportFreeze();

    bool isHitEnable() const;
    bool isSpinReactionEnable() const;
    bool isDeathEnable() const;
    bool isBindStarPointerEnable() const;
    bool tryDeath();
    bool tryHitReaction();
    bool trySpinReaction();
    bool tryStarPieceAttack();
    bool tryTongue();
    void updateOnRail();
    bool moveOnRailAndCheckNextPoint();

    /* 0x90 */ AnimScaleController* mAnimScaleController;
    /* 0x94 */ WalkerStateBindStarPointer* mBindStarPointer;
    /* 0x98 */ TVec3f mFrontVec;
    /* 0xA4 */ f32 mRailSpeed;
    /* 0xA8 */ YoshiLockOnTarget* mYoshiLockOn;
    /* 0xAC */ bool mIsGroupMoveStop;
};

class TereboGroup : public LiveActor {
public:
    TereboGroup(const char*);
    virtual ~TereboGroup();

    virtual void init(const JMapInfoIter&);
};
