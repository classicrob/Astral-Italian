#pragma once
#include <JSystem/JGeometry/TVec.hpp>
class LiveActor;
class YoshiLockOnTarget;
namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*,MtxPtr,const TVec3f&);
}
