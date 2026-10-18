#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include <JSystem/JGeometry.hpp>

class LiveActor;
class Nerve;

// PLACEHOLDER(objects): unnamed core helper = MR::requestStartDemo(NerveExecutor*, LiveActor*, const char*, const Nerve* pDemoNerve, const Nerve* pWaitNerve); resolve when core names fn_8001FEF0
extern "C" void fn_8001FEF0(NerveExecutor*, LiveActor*, const char*, const Nerve*, const Nerve*);

#include "Game/Util/ActorMovementUtil.hpp"  // MR::fn_80010CB0 (mVelocity += dir * f)

// PLACEHOLDER(objects): unnamed core helper (dst = src * rotation about src's X direction by f degrees); resolve when core names fn_8003D330
extern "C" void fn_8003D330(TQuat4f*, const TQuat4f&, f32);

class JMapInfoIter;
// PLACEHOLDER(objects): unnamed core helper (DemoDirector::registerDemoCast(actor, demoName, iter) + talk anim init for the demo); resolve when core names fn_80020860
extern "C" void fn_80020860(LiveActor*, const JMapInfoIter&, const char*, const char*);

// PLACEHOLDER(objects): unnamed core helper (value remap x in [min,max] -> [start,end] with easing); resolve when core names fn_8003B960
extern "C" f32 fn_8003B960(f32, f32, f32, f32, f32);
