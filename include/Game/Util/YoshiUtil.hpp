#pragma once
#include <JSystem/JGeometry.hpp>
class LiveActor;
class JMapInfoIter;
class YoshiLockOnTarget;
namespace MR {
    void endYoshiLightFruit();
    bool fn_80421D10();
    bool fn_80422150();
    void fn_80422190();
    void fn_804221D0();
    void fn_80422210();
    void fn_80422250();
    bool fn_80422290();
    bool fn_804222D0();
    bool fn_80422310();
    void fn_80422350(TPos3f*);
    f32 fn_80422390();
    void fn_804224F0();
    void fn_80422520();
    void fn_80422550();
    void fn_80422580();
    bool fn_804225C0();
    void startDashYoshiMode();
    bool isPlayerElementModeYoshiDash();
    bool isPlayerElementModeYoshiLight();
    bool fn_80422710();
    void fn_80422740();
    bool fn_80422880(u32);
    bool isMsgDashYoshiAttack(u32);
    void fn_804228B0();
    MtxPtr getYoshiMouthMtx();
    void incYoshiFruitEaten();

    void fn_804223C0(const JMapInfoIter&);
    void fn_80422400(LiveActor*);
    void fn_80422440(LiveActor*);
    void fn_80422480(LiveActor*);
    bool fn_804224C0();
    const TVec3f& fn_804226B0();
    const TVec3f& fn_804226E0();
    void fn_80422790(MtxPtr, LiveActor*);
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
    bool tryYoshiEat(YoshiLockOnTarget*);
    bool tryYoshiCapture(YoshiLockOnTarget*, bool);
}

namespace YoshiFunction { void emitGlowEffect(); void deleteGlowEffect(); }
