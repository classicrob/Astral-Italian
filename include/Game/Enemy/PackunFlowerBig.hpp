#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class WalkerStateBindStarPointer;

/// @brief PackunFlowerBig (SMG2 only, no SMG1 source). A big piranha plant that hides, appears, and can be frozen by a star pointer.
class PackunFlowerBig : public LiveActor {
public:
    PackunFlowerBig(const char* pName);
    static bool isSensorInAttackRange(HitSensor*, HitSensor*, TVec3f*, f32);

    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    void exeHidden();
    void exeAppear();
    void exeWait();
    void exeHide();
    void exeSpinReaction();
    void exeFreeze();
    void exeFreezeEnd();
    bool tryStarPointerBind();

    /* 0x90 */ AnimScaleController* mAnimScaleController;
    /* 0x94 */ WalkerStateBindStarPointer* mBindState;
    /* 0x98 */ s32 mTimer;
};
