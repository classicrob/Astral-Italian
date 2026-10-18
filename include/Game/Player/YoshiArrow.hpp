#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class YoshiArrow : public LiveActor {
public:
    void fn_80414040();
    void fn_80413DC0(const TVec3f&, const TVec3f&, const TVec3f&);
    void fn_80413FB0();
};
