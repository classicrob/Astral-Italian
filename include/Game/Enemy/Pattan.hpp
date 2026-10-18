#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class AnimScaleController;
class BreakModel;
class HitSensor;
class WalkerStateBindStarPointer;

// SMG2-only unit (Pattan, jumping flower-like enemy with a break model). LiveActor = 0x90 bytes.
class Pattan : public LiveActor {
public:
    Pattan(const char* pName);

    virtual ~Pattan();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);
    virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

    bool fn_801E7A80();
    bool fn_801E7B00();
    bool fn_801E7B90();
    bool fn_801E7C20();
    bool fn_801E7C70();
    bool fn_801E7D10(HitSensor*, HitSensor*);
    void fn_801E77A0();
    void fn_801E7030();
    void fn_801E70D0();
    void fn_801E7160();
    void fn_801E71D0();
    void exeAttackSign();
    void exeAttack();
    void fn_801E73B0();
    void fn_801E7470();
    void fn_801E7530();
    void fn_801E75B0();
    void fn_801E7670();
    void fn_801E7710();
    void exeFreeze();

public:
    /* 0x90 */ AnimScaleController* mAnimScaleCtrl;
    /* 0x94 */ WalkerStateBindStarPointer* mStarPointBind;
    /* 0x98 */ BreakModel* mBreakModel;
    /* 0x9C */ TPos3f mBaseMtx;
    /* 0xCC */ TVec3f mFront;
    /* 0xD8 */ s32 mTimer;
    /* 0xDC */ s32 mDamageTimer;
};

void appearPattanByJump(LiveActor*, const TVec3f*);
