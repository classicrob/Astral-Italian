#pragma once
#include "Game/Util/YoshiUtil.hpp"
namespace YoshiFunction {
    bool tryYoshiLick(YoshiLockOnTarget*);
    bool tryYoshiPull(YoshiLockOnTarget*);
    bool tryYoshiProjectile(YoshiLockOnTarget*);
    void emitGlowEffect();
    void deleteGlowEffect();
}
