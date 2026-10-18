#pragma once
#include "Game/MapObj/MapObjActor.hpp"
class PartsModel;
class MagmaBoatPlanetMagma : public MapObjActor {
public:
    MagmaBoatPlanetMagma(const char*);
    virtual ~MagmaBoatPlanetMagma();
    virtual void init(const JMapInfoIter&);
    virtual void connectToScene(const MapObjActorInitInfo&);
    virtual void initCaseUseSwitchA(const MapObjActorInitInfo&);
    virtual void initCaseUseSwitchB(const MapObjActorInitInfo&);
    void exeFreeze();
    PartsModel* mIceModel;
};
