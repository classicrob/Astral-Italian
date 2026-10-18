#pragma once

#include "Game/Map/PlanetMap.hpp"

class LiveActorGroup;

class KoopaBattlePlanet : public PlanetMap {
public:
    KoopaBattlePlanet(const char*);

    virtual ~KoopaBattlePlanet();
    virtual void init(const JMapInfoIter&);
    void initAfterPlacement();
    void makeActorAppeared();
    void makeActorDead();

    /* 0xB4 */ LiveActorGroup* mChildGroup;
};
