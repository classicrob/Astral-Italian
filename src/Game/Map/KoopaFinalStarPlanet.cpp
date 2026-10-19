#include "Game/Map/KoopaFinalStarPlanet.hpp"
#include "Game/Boss/KoopaBattleAccessor.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"
#include "Game/Util/MapUtil.hpp"

KoopaFinalStarPlanet::KoopaFinalStarPlanet(const char* pName) : PlanetMap(pName, nullptr) {}

void KoopaFinalStarPlanet::init(const JMapInfoIter& rIter) {
    MR::createKoopaBattleAccessor();
    MR::getKoopaBattleAccessor()->registerKoopaFinalStarPlanet(this);
    PlanetMap::init(rIter);
    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, nullptr);
    makeActorDead();
}

KoopaFinalStarPlanet::~KoopaFinalStarPlanet() {}
