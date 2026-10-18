#pragma once

#include "Game/LiveActor/LiveActor.hpp"

/// Rocket the player rides on the world map (0xEC bytes, ctor 0x804E4530).
class MiniPlayerRocket : public LiveActor {
public:
    MiniPlayerRocket();

    virtual ~MiniPlayerRocket();
    virtual void init(const JMapInfoIter&);

    /* 0x90 */ u8 _90[0x5C];
};
