#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class ItemBlock : public LiveActor {
public:
    ItemBlock(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void calcAnim();
    virtual void makeActorAppeared();
    virtual void kill();
    virtual void startClipped();
    virtual void endClipped();
    virtual void control();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);

    void checkKind(const JMapInfoIter&);
    bool tryStartJumpPunch();
    void exeStandby();
    void exeWait();
    void exeAppearItem();
    void exeAppearItemSplash();
    void decTimer();

    /* 0x90 */ s32 mKind;
    /* 0x94 */ TVec3f mClippingRange;
    /* 0xA0 */ s32 mTimer;
    /* 0xA4 */ s32 mCooldownTimer;
    /* 0xA8 */ s32 mItemCount;
    /* 0xAC */ u8 _AC;
    /* 0xAD */ bool mCalcShadow;
    /* 0xAE */ bool mNoCalcAnim;
};
