#pragma once
#include <JSystem/JGeometry/TVec.hpp>
class LiveActor;
class YoshiLockOnTarget;
namespace MR {
    bool isMsgYoshiAutoEatStart(u32);
    bool isMsgYoshiAutoEatEnd(u32);
    // PLACEHOLDER(m1): predicate tests message 0xFE; resolve when Yoshi message semantics are recovered.
    bool fn_80422890(u32);
    void createLightFruitClipArea();
    void startYoshiLightFruit();
    bool requestYoshiCaptureGuidance();  // returns bool (StarPointerUtil.cpp, asm)
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*,const char*,const TVec3f&);
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*,MtxPtr,const TVec3f&);
}

namespace YoshiFunction {
    bool tryYoshiPushEat();
    void tryYoshiEat(YoshiLockOnTarget*);
    void tryYoshiCapture(YoshiLockOnTarget*, bool);
}

namespace YoshiFunction { void emitGlowEffect(); void deleteGlowEffect(); }
