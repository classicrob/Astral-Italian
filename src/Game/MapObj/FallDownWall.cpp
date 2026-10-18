#pragma auto_inline off
#include "Game/MapObj/FallDownWall.hpp"
#include "Game/MapObj/StageEffectDataTable.hpp"
#include "Game/Util.hpp"
namespace {
TVec3f sShadowOffset(-200.0f, -13000.0f, 150.0f);
}
FallDownWall::FallDownWall(const char* name) : RailMoveObj(name) { mShadowMtx.identity(); }
void FallDownWall::init(const JMapInfoIter& iter) {
    RailMoveObj::init(iter);
    mShadowMtx.set(getBaseMtx());
    MR::setShadowDropPositionMtxPtr(this, "Body", mShadowMtx, sShadowOffset);
}
void FallDownWall::startMoveInner() { RailMoveObj::startMoveInner(); MR::emitEffect(this, "MoveStart"); }
void FallDownWall::doAtEndPoint() {
    TVec3f toPlayer(0.0f), front(0.0f);
    MR::calcDirToPlayer(&toPlayer, this);
    MR::calcFrontVec(&front, this);
    if (toPlayer.dot(front) >= 0.0f) MR::StageEffect::tryStageEffectStop(this, mObjectName);
    if (MR::isExistBck(this, "Move") && MR::isBckPlaying(this, "Move")) MR::stopBck(this);
    MR::StageEffect::stopShakingCameraMoving(this, mObjectName);
}
FallDownWall::~FallDownWall() {}
void RailMoveObj::setupInitInfo(const JMapInfoIter&, MapObjActorInitInfo*) {}
