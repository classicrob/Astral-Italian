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

    /* 0xB0 */ u8 _B0[0x4];  // not yet mapped (PlanetMap is 0xB0 bytes)
    /* 0xB4 */ LiveActorGroup* mChildGroup;
};
