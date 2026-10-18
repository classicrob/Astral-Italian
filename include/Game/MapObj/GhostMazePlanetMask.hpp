#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class GhostMazePlanetMask : public LiveActor {
public:
    GhostMazePlanetMask(const char*);
    virtual ~GhostMazePlanetMask();
    virtual void init(const JMapInfoIter&);
    void exeWait();
    void exeAppear();
    void exeDisappear();
};
