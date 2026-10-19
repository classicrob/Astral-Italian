#include "Game/LiveActor/ChangeObj.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"

void ChangeObj::changeTRSVGFromOther(const ChangeObj* pOther) {
    MR::setRotation(mActor, pOther->mActor->mRotation);
    MR::setScale(mActor, pOther->mActor->mScale);
    mActor->mVelocity.set(pOther->mActor->mVelocity);
    mActor->mGravity.set(pOther->mActor->mGravity);
    MR::setPosition(mActor, pOther->mActor->mPosition);
    MR::resetPosition(mActor);
}
