#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class VsOtaRockTankIcePlanetFence : public LiveActor {
public:
    VsOtaRockTankIcePlanetFence(const char*);
    virtual ~VsOtaRockTankIcePlanetFence();
    virtual void init(const JMapInfoIter&);
};
