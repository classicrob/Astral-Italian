#pragma once

#include "Game/LiveActor/Nerve.hpp"

namespace NrvDreamer {
#define DECLARE_DREAMER_NERVE(State) \
    class DreamerNrv##State : public Nerve { \
    public: DreamerNrv##State(); virtual void execute(Spine*) const; \
    static DreamerNrv##State sInstance; };
    DECLARE_DREAMER_NERVE(Appear)
    DECLARE_DREAMER_NERVE(StartHelp)
    DECLARE_DREAMER_NERVE(InHelp)
    DECLARE_DREAMER_NERVE(EndHelp)
#undef DECLARE_DREAMER_NERVE
}
