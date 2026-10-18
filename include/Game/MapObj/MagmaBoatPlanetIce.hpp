#pragma once
#include "Game/MapObj/MapObjActor.hpp"
class PartsModel;
class MagmaBoatPlanetIce : public MapObjActor {
public:
    MagmaBoatPlanetIce(const char*);
    virtual ~MagmaBoatPlanetIce();
    virtual void init(const JMapInfoIter&);
    virtual void connectToScene(const MapObjActorInitInfo&);
    void exeFreeze();
    PartsModel* mIceModel;
};
