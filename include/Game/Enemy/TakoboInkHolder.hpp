#pragma once

#include "Game/LiveActor/LiveActor.hpp"

// Minimal declaration from its uses (TakoSho): allocated as 0x20 bytes, built by a default ctor,
// spurts ink with (pos, dir). Its base is DeriveActorGroup<TakoboInk> (not modeled here).
class TakoboInkHolder {
public:
    TakoboInkHolder();

    void spurtTakoboInk(const TVec3f&, const TVec3f&);

    /* 0x00 */ u8 _00[0x20];
};
