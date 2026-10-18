#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/System/NerveExecutor.hpp"

class AnimScaleController;

/// @brief Star pointer touch helper (SMG2 SupportTicoStop). Only the parts used by enemies are declared.
class SupportTicoStop : public NerveExecutor {
public:
    SupportTicoStop(LiveActor*, AnimScaleController*);

    bool tryAttachTouch();
    bool isTouching() const;

    /* 0x08 */ u8 _8[0xD];
    /* 0x15 */ bool mIsTouching;
    /* 0x16 */ u8 _16[0x2];
};
