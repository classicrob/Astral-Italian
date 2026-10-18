#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class MysteryMountainPlanet : public LiveActor {
public:
    MysteryMountainPlanet(const char*);
    virtual ~MysteryMountainPlanet();
    virtual void init(const JMapInfoIter&);
    void exeBoxVanish();
    void exeCakeVanish();
};
