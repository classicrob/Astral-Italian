#pragma once
#include "Game/MapObj/MapObjActor.hpp"
class QuicksandPlanet : public MapObjActor {
public:
    QuicksandPlanet(const char*);
    virtual ~QuicksandPlanet();
    virtual void init(const JMapInfoIter&);
    virtual void makeActorAppeared();
    virtual void startClipped();
    virtual void endClipped();
    bool isPlayerMoveable();
    s32 mEffectCount;
};
