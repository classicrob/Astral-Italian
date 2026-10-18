#pragma once
#include "Game/MapObj/RailMoveObj.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class FallDownWall : public RailMoveObj {
public:
    FallDownWall(const char*);
    virtual ~FallDownWall();
    virtual void init(const JMapInfoIter&);
    virtual void startMoveInner();
    virtual void doAtEndPoint();
    TPos3f mShadowMtx;
};
