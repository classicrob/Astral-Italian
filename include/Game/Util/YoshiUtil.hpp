#pragma once
#include <JSystem/JGeometry/TVec.hpp>
class LiveActor;
class YoshiLockOnTarget;
namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*,const char*,const TVec3f&);
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*,MtxPtr,const TVec3f&);
}

namespace YoshiFunction {
    void tryYoshiEat(YoshiLockOnTarget*);
}
