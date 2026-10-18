#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
class PoleObstacleGroup : public LiveActorGroup {
public:
    PoleObstacleGroup(const char* name,s32 count) : LiveActorGroup(name,count) {}
    virtual ~PoleObstacleGroup();
};
class RailPlacementPoleObstacle : public LiveActor {
public:
    RailPlacementPoleObstacle(const char*);
    virtual ~RailPlacementPoleObstacle();
    virtual void init(const JMapInfoIter&);
    PoleObstacleGroup* mGroup;
    f32 mSpacing;
    s32 mModelType;
};
