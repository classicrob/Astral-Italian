#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class CrescentSeesawPlanet : public LiveActor {
public:
    CrescentSeesawPlanet(const char*);
    virtual ~CrescentSeesawPlanet();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void calcAndSetBaseMtx();
    void move();
    TVec3f mAxis;
    f32 mAngularVelocity;
    f32 mAngle;
    TPos3f mInitialMtx;
};
