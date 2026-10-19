#pragma once

#include "Game/Util/YoshiUtil.hpp"

namespace YoshiFunction {
    bool tryYoshiPushEat();
    bool tryYoshiEat(YoshiLockOnTarget* target);
    bool tryYoshiLick(YoshiLockOnTarget* target);
    bool tryYoshiCapture(YoshiLockOnTarget* target, bool alternateCapture);
    bool tryYoshiPull(YoshiLockOnTarget* target);
    bool tryYoshiProjectile(YoshiLockOnTarget* target);
    void emitGlowEffect();
    void deleteGlowEffect();
}
