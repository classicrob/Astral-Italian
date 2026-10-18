#pragma once

#include "Game/NameObj/NameObj.hpp"

class AudBgmConductor : public NameObj {
public:
    /// @brief Creates a new `AudBgmConductor`.
    AudBgmConductor();

    virtual void init(const JMapInfoIter& rIter);
    virtual void movement();

    void playPinchEventBgmLevel(bool);
    void setStateBit(u32);
    void stopPinchEventBgmSoon();
    void movementPinchEventBgm();
    void movementMorphBgm();

    // TODO: clarify StateBit_1
    enum StateBits {
        StateBit_1 = 0b010,
        StateBit_Delay = 0b100,
    };

    /* 0x14 */ s32 _14;  // SMG2-only, -1 at construction (used by movementMorphBgm)
    /* 0x18 */ u32 mStateBits;
    /* 0x1C */ s32 mState;
    /* 0x20 */ s32 mStartDelay;
    /* 0x24 */ s32 mFadeInDelay;
    /* 0x28 */ bool mStopImmediately;
};

namespace AudBgmConductorFunction {
    s32 updatePlayerState(u32*, s32, u32) NO_INLINE;
    s32 updateSwitchState(u32*, s32, u32) NO_INLINE;
}
