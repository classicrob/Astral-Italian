#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class AnimScaleController;
class WalkerStateBindStarPointer;

// SMG2 unnamed helper at 0x801E1C60 (no owner split assigned yet), named by address; used by PackunFlowerBig attackSensor.
s32 fn_801E1C60(HitSensor* pSender, HitSensor* pReceiver, TVec3f* pDir, f32 scale);

/// @brief PackunFlowerBig (SMG2 only, no SMG1 source). A big piranha plant that hides, appears, and can be frozen by a star pointer.
class PackunFlowerBig : public LiveActor {
public:
    PackunFlowerBig(const char* pName);

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

    /* 0x44 */ TVec3f mUnk44;
    /* 0x50 */ u8 _50[0x40];
    /* 0x90 */ AnimScaleController* mAnimScaleController;
    /* 0x94 */ WalkerStateBindStarPointer* mBindState;
    /* 0x98 */ s32 mTimer;
};
