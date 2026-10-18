#pragma once
#include "Game/MapObj/MapObjActor.hpp"
class RelayDrill2DPlanet : public MapObjActor {
public:
    RelayDrill2DPlanet(const char*);
    virtual ~RelayDrill2DPlanet();
    virtual void init(const JMapInfoIter&);
    virtual void initCaseUseSwitchA(const MapObjActorInitInfo&);
    void validateCollision();
};
