#pragma once

#include "Game/LiveActor/LiveActor.hpp"

// Minimal declaration (SMG2 only, not ported yet): enough for GliBird to `new` it (size 0x164).
class GliBirdNpc : public LiveActor {
public:
    GliBirdNpc(const char*);

    u8 _90[0x164 - 0x90];
};
