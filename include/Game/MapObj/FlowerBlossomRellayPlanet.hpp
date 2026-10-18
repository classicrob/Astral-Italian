#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class FlowerBlossomRellayPlanet : public LiveActor {
public:
    FlowerBlossomRellayPlanet(const char*);
    virtual ~FlowerBlossomRellayPlanet();
    virtual void init(const JMapInfoIter&);
};
