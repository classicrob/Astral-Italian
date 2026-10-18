#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry.hpp>
class MarioFacePlanetPrevious : public LiveActor {
public:
    MarioFacePlanetPrevious(const char*);
    virtual ~MarioFacePlanetPrevious();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    void exeDemo();
    TPos3f mScreenMtx; // 0x90
    TPos3f mScreenLightMtx; // 0xC0
};
