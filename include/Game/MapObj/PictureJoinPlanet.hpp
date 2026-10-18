#pragma once
#include "Game/MapObj/SimpleMapObj.hpp"
class PictureJoinPlanet : public SimpleMapObj {
public:
    PictureJoinPlanet(const char* name) : SimpleMapObj(name) {}
    virtual ~PictureJoinPlanet();
    virtual void init(const JMapInfoIter&);
};
