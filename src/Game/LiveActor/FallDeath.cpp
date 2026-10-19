#include "Game/LiveActor/FallDeath.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"

namespace NrvFallDeath {
    FallDeathNrvFall FallDeathNrvFall::sInstance ATTRIBUTE_WEAK;

    void FallDeathNrvFall::execute(Spine* pSpine) const {
        FallDeath* pActor = reinterpret_cast< FallDeath* >(pSpine->mExecutor);
        MR::addVelocityToGravity(pActor, *pActor->mFallAccel);
    }
};  // namespace NrvFallDeath

FallDeath::~FallDeath() {
}

void FallDeath::init(const JMapInfoIter& rIter) {
    initNerve(&NrvFallDeath::FallDeathNrvFall::sInstance, 0);
    MR::invalidateClipping(this);
    makeActorAppeared();
}
