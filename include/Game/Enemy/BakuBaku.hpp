#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ActorPoseTQ;
class AnimScaleController;
class WalkerStateBindStarPointer;
class HitSensor;

// SMG2-only unit (BakuBaku: rail-riding biting plant enemy). LiveActor = 0x90 bytes.
class BakuBaku : public LiveActor {
public:
    // SMG2-only functions (original names unknown; named by address)
    bool fn_80155F00();
    bool fn_80155F60();
    BakuBaku(const char* pName);

    virtual ~BakuBaku();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual void control();
    virtual void calcAndSetBaseMtx();

    bool isAttackable() const;
    void exeTurn();
    void exeMove();
    void exeSuccess();

public:
    /* 0x90 */ ActorPoseTQ* mPose;
    /* 0x94 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x98 */ WalkerStateBindStarPointer* mStarPointBind;
    /* 0x9C */ TVec3f _9C;
    /* 0xA8 */ f32 mMoveLimit;
};
