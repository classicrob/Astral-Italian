#pragma once

#include "Game/LiveActor/LiveActorGroup.hpp"

/// @brief Holds the pool of 0x40 GliderShooterBullet actors (SMG2 only).
class GliderShooterBulletHolder : public LiveActorGroup {
public:
    GliderShooterBulletHolder();

    void init(const JMapInfoIter& rIter);
};
