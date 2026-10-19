#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class HitSensor;
class WalkerStateBindStarPointer;

// SMG2-only unit (Bussun, rail-walking enemy that turns at rail ends and drops a coin). LiveActor = 0x90 bytes.
class Bussun : public LiveActor {
public:
    Bussun(const char* pName);

    virtual ~Bussun();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void kill();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    const Nerve* getBussunContactReactionNerve() const;
    const Nerve* getBussunDamageReactionNerve() const;
    NO_INLINE bool canBussunReceiveAttack() const;
    NO_INLINE bool canBussunAttackPlayer() const;
    bool canBussunBindStarPointer() const;
    void selectBussunPointBindReturnNerve();
    bool isBussunOnMetalGround() const;
    void calcBussunRailDirection(TVec3f*);
    void exeBussunWalk();
    void exeWait();
    void exeTurn();
    void exeBussunTurnEnd();
    void exeAttack();
    void exeHit();
    void exeDown();
    void exeBlow();
    void exeBussunDead();
    void exeFreeze();

public:
    /* 0x90 */ TVec3f mFront;
    /* 0x9C */ TVec3f mUp;
    /* 0xA8 */ s32 mRailPointNo;
    /* 0xAC */ AnimScaleController* mAnimScaleCtrl;
    /* 0xB0 */ WalkerStateBindStarPointer* mStarPointBind;
    /* 0xB4 */ const Nerve* mNextNerve;
};
