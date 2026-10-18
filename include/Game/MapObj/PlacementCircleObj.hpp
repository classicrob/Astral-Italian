#pragma once
#include "Game/MapObj/PlacementObj.hpp"
class PlacementCircleObj : public PlacementObj {
public:
    PlacementCircleObj();
    virtual ~PlacementCircleObj();
    virtual void init(const JMapInfoIter&);
    virtual void calcPosition(s32,s32,TVec3f*);
    TVec3f mRight;
    TVec3f mFront;
    f32 mRadius;
};
