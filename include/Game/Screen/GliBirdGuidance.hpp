#pragma once

#include "Game/Screen/LayoutActor.hpp"

// Minimal declaration (SMG2 only, not ported yet): enough for GliBirdTutorial (size 0x30).
class GliBirdGuidance : public LayoutActor {
public:
    GliBirdGuidance(const char*);

    /* 0x2C */ s32 mMode;
};
