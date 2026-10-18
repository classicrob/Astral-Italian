#include "Game/NPC/EatDemoCoin.hpp"
#include "Game/Util.hpp"

EatDemoCoin::EatDemoCoin(const char* name) : LiveActor(name), mStartPosition(0.0f, 0.0f, 0.0f), mTargetPosition(nullptr) {
    mQuat.set<f32>(0.0f, 0.0f, 0.0f, 1.0f);
    MR::initActor(this, JMapInfoIter(), "Coin", false);
    MR::connectToSceneItemStrongLight(this);
    MR::invalidateClipping(this);
    initNerve(&NrvEatDemoCoin::EatDemoCoinNrvShoot::sInstance);
    makeActorDead();
}

#pragma dont_inline on
void EatDemoCoin::shoot(const TVec3f& start, const TVec3f* target) {
    MR::tryRumblePadWeak(this, 0);
    mStartPosition.set<f32>(start);
    mTargetPosition = target;
    mPosition.set<f32>(mStartPosition);
    mScale.setAll<f32>(0.8f);
    TVec3f direction(0.0f, 0.0f, 1.0f);
    direction.sub(*target, start);
    PSVECMag(&direction);
    PSVECNormalize(&direction, &direction);
    MR::makeQuatUpFront(&mQuat, MR::getCamYdir(), direction);
    MR::requestMovementOn(this);
    makeActorAppeared();
    setNerve(&NrvEatDemoCoin::EatDemoCoinNrvShoot::sInstance);
}

#pragma dont_inline reset

void EatDemoCoin::exeShoot() {
    MR::vecBlend(&mPosition, mStartPosition, *mTargetPosition, MR::calcNerveRate(this, 20));
    MR::rotateQuatLocalYDegree(&mQuat, mQuat, 30.0f);
    if (MR::isStep(this, 20)) {
        kill();
    }
}

void EatDemoCoin::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
}
