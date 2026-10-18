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

    const Nerve* fn_80175850() const;
    const Nerve* fn_801758B0() const;
    NO_INLINE bool fn_80175910() const;
    NO_INLINE bool fn_80175940() const;
    bool fn_80175970() const;
    void fn_801759D0();
    bool fn_80175A70() const;
    void fn_801757C0(TVec3f*);
    void fn_80175330();
    void exeWait();
    void exeTurn();
    void fn_801754E0();
    void exeAttack();
    void exeHit();
    void exeDown();
    void exeBlow();
    void fn_801756F0();
    void exeFreeze();

public:
    /* 0x90 */ TVec3f mFront;
    /* 0x9C */ TVec3f mUp;
    /* 0xA8 */ s32 mRailPointNo;
    /* 0xAC */ AnimScaleController* mAnimScaleCtrl;
    /* 0xB0 */ WalkerStateBindStarPointer* mStarPointBind;
    /* 0xB4 */ const Nerve* mNextNerve;
};
