#include "Game/MapObj/PictureJoinPlanet.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
PictureJoinPlanet::~PictureJoinPlanet() {}
void PictureJoinPlanet::init(const JMapInfoIter& iter) {
    MapObjActor::init(iter);
    MapObjActorInitInfo info;
    MapObjActorUtil::setupInitInfoSimpleMapObj(&info);
    MapObjActorUtil::setupInitInfoTypical(&info,mObjectName);
    MapObjActorUtil::setupInitInfoColorChangeArg0(&info,iter);
    MapObjActorUtil::setupInitInfoTextureChangeArg1(&info,iter);
    MapObjActorUtil::setupInitInfoShadowLengthArg2(&info,iter);
    info.setupBaseMtxFollowTarget();
    initialize(iter,info);
}
