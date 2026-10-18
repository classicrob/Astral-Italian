#pragma once

#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Enemy/TakoboInk.hpp"

/// @brief Pool of TakoboInk actors (SMG2). Built with a count by TakoSho (count 2 in its asm); spurts ink from the dead pool.
class TakoboInkHolder : public DeriveActorGroup< TakoboInk > {
public:
    TakoboInkHolder(s32 numMax);

    bool spurtTakoboInk(const TVec3f& rPos, const TVec3f& rDir);
};
