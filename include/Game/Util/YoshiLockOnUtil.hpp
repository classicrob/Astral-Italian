#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class YoshiLockOnTarget;

namespace YoshiFunction {
    bool tryYoshiEat(YoshiLockOnTarget*);
    bool tryYoshiLick(YoshiLockOnTarget*);
};  // namespace YoshiFunction

namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, MtxPtr, const TVec3f&);
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, const char*, const TVec3f&);
};  // namespace MR
