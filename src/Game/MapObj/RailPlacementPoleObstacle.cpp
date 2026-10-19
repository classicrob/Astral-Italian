#include "Game/MapObj/RailPlacementPoleObstacle.hpp"
#include "Game/MapObj/PoleObstacleParts.hpp"
#include "Game/Util.hpp"
namespace {
    const char* sModels[]={"PoleObstaclePartsA","PoleObstaclePartsB","PoleObstaclePartsC","PoleObstaclePartsD"};
}
RailPlacementPoleObstacle::RailPlacementPoleObstacle(const char* name) : LiveActor(name),mSpacing(100.0f),mModelType(0) {}
RailPlacementPoleObstacle::~RailPlacementPoleObstacle() {}
PoleObstacleGroup::~PoleObstacleGroup() {}
void RailPlacementPoleObstacle::init(const JMapInfoIter& iter) {
    MR::getJMapInfoArg0NoInit(iter,&mSpacing);
    MR::getJMapInfoArg1NoInit(iter,&mModelType);
    initRailRider(iter);
    s32 count=MR::getRailTotalLength(this)/mSpacing;
    mGroup=new PoleObstacleGroup("レール配置ポール障害物",count);
    mGroup->initWithoutIter();
    for (s32 i=0;i<count;i++) {
        PoleObstacleParts* actor=new PoleObstacleParts("ポール障害物",this,sModels[mModelType]);
        actor->initWithoutIter();
        TVec3f pos(0.0f);
        MR::calcRailPosAtCoord(&pos,this,mSpacing*i);
        MR::setPosition(actor,pos);
        mGroup->registerActor(actor);
    }
    MR::invalidateClipping(this);
    makeActorAppeared();
}
