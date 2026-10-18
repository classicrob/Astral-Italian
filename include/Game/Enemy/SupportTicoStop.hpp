#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/System/NerveExecutor.hpp"

class AnimScaleController;

// SMG2 unnamed helper at 0x80203850 (SupportTicoStop.cpp), named by address: turns pDst toward the player when pTarget is near.
void fn_80203850(TVec3f* pDst, const LiveActor* pTarget);

/// @brief Star pointer touch helper (SMG2 SupportTicoStop): attaches a SupportTico to a target actor while it is touched.
class SupportTicoStop : public NerveExecutor {
public:
    SupportTicoStop(LiveActor*, AnimScaleController*);

    bool tryAttachTouch();
    bool tryTouch();
    bool isTouching() const;

    void exeTouch();
    void exeEndTouch();
    void exeDoNothing();

    /* 0x08 */ LiveActor* mHost;
    /* 0x0C */ AnimScaleController* mScaleController;
    /* 0x10 */ s32 mCounter;
    /* 0x14 */ bool mIsTouchStarted;
    /* 0x15 */ bool mIsTouching;
    /* 0x16 */ bool mHasPointerEffect;
};
