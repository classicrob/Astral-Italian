#include "Game/MapObj/VsOtaRockTankIcePlanetFence.hpp"
#include "Game/Util.hpp"
VsOtaRockTankIcePlanetFence::VsOtaRockTankIcePlanetFence(const char* name) : LiveActor(name) {}
VsOtaRockTankIcePlanetFence::~VsOtaRockTankIcePlanetFence() {}
void VsOtaRockTankIcePlanetFence::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    makeActorAppeared();
}
