#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class MarioFacePlanet : public LiveActor {
public:
    MarioFacePlanet(const char*);
    virtual ~MarioFacePlanet();
    virtual void init(const JMapInfoIter&);
    void listenDemoAction();
};
