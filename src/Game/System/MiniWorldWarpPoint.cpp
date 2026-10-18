#include "Game/System/MiniPartsActors.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

void MiniWorldWarpPoint::init(const JMapInfoIter& rIter) {
    MR::initActor(this, "MiniWorldWarpPoint", "WMap", false);
    makeActorAppeared();
}

void MiniWorldWarpPoint::makeActorAppeared() {
    if (mWorld->hasOpenLineNoAlpha(mPointIndex)) {
        LiveActor::makeActorAppeared();
        const char* pActionName = mIsForward ? "Next" : "Previous";
        MR::startAction(this, pActionName);
    }
}

void MiniWorldWarpPoint::makeActorDead() {
    LiveActor::makeActorDead();
    MR::forceDeleteEffectAll(this);
}

bool MiniWorldWarpPoint::updateDistance(s32 distance) {
    if (MR::isDead(this)) {
        return false;
    }

    if (distance >= 2) {
        return false;
    }

    MR::startActionSound(this, "MarioShipBesideThis", -1, -1, -1);
    return true;
}

void MiniWorldWarpPoint::setEffectDrawOrder(s32 order) {
    MR::setAllEffectDrawOrder(this, order);
}

MiniWorldWarpPoint::~MiniWorldWarpPoint() {
}
