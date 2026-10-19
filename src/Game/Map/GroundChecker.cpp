#include "Game/Map/GroundChecker.hpp"
#include "Game/Util/LiveActorUtil.hpp"

GroundChecker::GroundChecker(const char* pName, f32 a2, f32 a3) : LiveActor(pName), mFrictionRate(1.0f), mUnusedFriction() {
    MR::invalidateClipping(this);
    initBinder(a2, a3, 0);
    makeActorAppeared();
}

void GroundChecker::bringFriction(f32 a1, f32 a2) {
    mFrictionRate = mFrictionRate * (1.0f - a2) + (a1 * a2);
}
