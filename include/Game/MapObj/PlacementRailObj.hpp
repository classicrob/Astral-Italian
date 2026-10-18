#pragma once
#include "Game/MapObj/PlacementObj.hpp"
class PlacementRailObj : public PlacementObj {
public:
    PlacementRailObj();
    virtual void init(const JMapInfoIter&);
    virtual void calcPosition(s32, s32, TVec3f*);
};
