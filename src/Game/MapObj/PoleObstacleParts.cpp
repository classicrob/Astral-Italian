#pragma auto_inline off
#include "Game/MapObj/PoleObstacleParts.hpp"
#include "Game/Util.hpp"
#include "Game/Map/HitInfo.hpp"
PoleObstacleParts::PoleObstacleParts(const char* name, const LiveActor* placementActor, const char* modelName) : LiveActor(name), mPlacementActor(placementActor), mModelName(modelName), mAlignToGravity(false) {}
void PoleObstacleParts::init(const JMapInfoIter& iter) {
    if (mPlacementActor && mModelName) mAlignToGravity = true;
    if (mAlignToGravity) MR::initActor(this, iter, mModelName, false);
    else MR::initActor(this, iter, false);
    MR::onCalcGravity(this);
    makeActorAppeared();
}
void PoleObstacleParts::initAfterPlacement() {
    if (mAlignToGravity) {
        MR::calcGravity(this);
        TVec3f direction = mGravity * 200.0f;
        TVec3f start(mPosition - mGravity * 5.0f);
        TVec3f position(0.0f);
        Triangle triangle;
        if (MR::getFirstPolyOnLineToMapExceptActor(&position, &triangle, start, direction, this)) mPosition = position;
    }
}
void PoleObstacleParts::calcAndSetBaseMtx() {
    if (!mAlignToGravity) { LiveActor::calcAndSetBaseMtx(); return; }
    TPos3f mtx;
    mtx.identity();
    TVec3f up = -mGravity;
    MR::makeMtxUpNoSupportPos(&mtx, up, mPosition);
    MR::setBaseTRMtx(this, mtx);
}
PoleObstacleParts::~PoleObstacleParts() {}
