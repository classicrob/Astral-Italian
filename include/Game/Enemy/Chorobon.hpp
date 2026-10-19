#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class HitSensor;
class WalkerStateBindStarPointer;

// SMG2-only unit (Chorobon, rail-riding bird enemy). LiveActor = 0x90 bytes.
class Chorobon : public LiveActor {
public:
    // SMG2-only functions (original names unknown; named by address)
    bool isAttackableChorobonNerve();
    bool isAliveChorobonNerve() NO_INLINE;
    bool isNotKillNerve() NO_INLINE;
    bool isStarBindableNerve();
    Chorobon(const char* pName);

    virtual ~Chorobon();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeWait();
    void exeMove();
    void exeAttack();
    void exeReaction();
    void exeKill();

public:
    /* 0x90 */ TVec3f mRailClip;
    /* 0x9C */ f32 mMoveSpeed;
    /* 0xA0 */ AnimScaleController* mAnimScaleCtrl;
    /* 0xA4 */ WalkerStateBindStarPointer* mStarPointBind;
};
