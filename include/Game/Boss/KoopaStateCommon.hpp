#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"
#include <JSystem/JGeometry.hpp>

class LiveActor;
class Nerve;

// PLACEHOLDER(objects): unnamed core helper = MR::requestStartDemo(NerveExecutor*, LiveActor*, const char*, const Nerve* pDemoNerve, const Nerve* pWaitNerve); resolve when core names fn_8001FEF0
extern "C" void fn_8001FEF0(NerveExecutor*, LiveActor*, const char*, const Nerve*, const Nerve*);

#include "Game/Util/ActorMovementUtil.hpp"  // MR::fn_80010CB0 (mVelocity += dir * f)

class JMapInfoIter;
// PLACEHOLDER(objects): unnamed core helper (DemoDirector::registerDemoCast(actor, demoName, iter) + talk anim init for the demo); resolve when core names fn_80020860
extern "C" void fn_80020860(LiveActor*, const JMapInfoIter&, const char*, const char*);
