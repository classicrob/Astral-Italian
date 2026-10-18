#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class WorldWarpPoint : public LiveActor {
public:
    WorldWarpPoint(const char*);
    virtual ~WorldWarpPoint();
    virtual void init(const JMapInfoIter&);
};
