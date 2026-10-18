#pragma once

#include "Game/Map/PlanetMap.hpp"

class KoopaFinalStarPlanet : public PlanetMap {
public:
    KoopaFinalStarPlanet(const char*);
    virtual ~KoopaFinalStarPlanet();
    virtual void init(const JMapInfoIter&);
};
