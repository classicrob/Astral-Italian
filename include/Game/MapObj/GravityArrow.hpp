#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class GravityArrow : public LiveActor {
public:
    GravityArrow(const char*);
    virtual ~GravityArrow();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual void endClipped();
    TPos3f mBaseMtx;
    TVec3f mFront;
    TVec3f mUp;
};
