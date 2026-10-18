#include "Game/MapObj/WireCoverCage.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/Util.hpp"
WireCoverCage::~WireCoverCage() {}
void WireCoverCage::init(const JMapInfoIter& iter) {
    MapObjActor::init(iter);
    MapObjActorInitInfo info;
    MapObjActorUtil::setupInitInfoSimpleMapObj(&info);
    MapObjActorUtil::setupInitInfoTypical(&info,mObjectName);
    initialize(iter,info);
    bool drill=false;
    MR::getJMapInfoArg0NoInit(iter,&drill);
    if (drill) {
        MR::invalidateCollisionParts(this);
        mDrillCollision=MR::createCollisionPartsFromLiveActor(this,"WireCoverCageDrill",getSensor("Body"),MR::CollisionScaleType(2));
        MR::validateCollisionParts(mDrillCollision);
    }
}
