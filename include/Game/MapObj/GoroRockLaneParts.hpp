#pragma once
#include "Game/MapObj/MapObjActor.hpp"
class GoroRockLaneParts : public MapObjActor {
public:
    GoroRockLaneParts(const char*);
    virtual ~GoroRockLaneParts();
    virtual void init(const JMapInfoIter&);
    virtual void initCaseUseSwitchB(const MapObjActorInitInfo&);
    virtual void initCaseUseSwitchA(const MapObjActorInitInfo&);
    void setChangeColor();
    void setNormalColor();
    TVec3f mClippingCenter;
};
