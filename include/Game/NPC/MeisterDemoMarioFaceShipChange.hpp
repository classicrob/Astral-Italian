#pragma once
#include "Game/System/NerveExecutor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class Meister;
class JMapInfoIter;
class DemoObj;
class MeisterDemoMarioFaceShipChange : public NerveExecutor {
public:
    MeisterDemoMarioFaceShipChange(Meister*, const JMapInfoIter&);
    virtual ~MeisterDemoMarioFaceShipChange();
    void start();
    void exeWait();
    Meister* mHost;
    DemoObj* mStar;
    TPos3f mStarMtx;
    f32 mUnused40;
};
