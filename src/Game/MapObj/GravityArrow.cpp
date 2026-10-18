#pragma auto_inline off
#include "Game/MapObj/GravityArrow.hpp"
#include "Game/Util.hpp"
#pragma auto_inline on
GravityArrow::GravityArrow(const char* name) : LiveActor(name), mFront(0.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f) { PSMTXIdentity(mBaseMtx); }
#pragma auto_inline off
void GravityArrow::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, "GravityArrow", false);
    MR::initRotation(&mFront, iter);
    MR::calcUpVec(&mUp, this);
    MR::makeMtxUpFrontPos(&mBaseMtx, mUp, mFront, mPosition);
    makeActorAppeared();
}
void GravityArrow::calcAndSetBaseMtx() { MR::setBaseTRMtx(this, mBaseMtx); }
void GravityArrow::control() {
    MR::calcFrontVec(&mFront, this);
    MR::calcUpVec(&mUp, this);
    TVec3f up = -*MR::getPlayerGravity();
    if (MR::isNearZero(1.0f + mUp.dot(up), 0.001f)) { TVec3f side; MR::calcSideVec(&side, this); up = side; }
    MR::vecBlend(&mUp, mUp, up, 0.1f);
    MR::normalize(&mUp);
    MR::makeMtxUpFrontPos(&mBaseMtx, mUp, mFront, mPosition);
}
void GravityArrow::endClipped() {
    MR::calcFrontVec(&mFront, this);
    LiveActor::endClipped();
    mUp = -*MR::getPlayerGravity();
    MR::makeMtxUpFrontPos(&mBaseMtx, mUp, mFront, mPosition);
    MR::setBaseTRMtx(this, mBaseMtx);
}
GravityArrow::~GravityArrow() {}
