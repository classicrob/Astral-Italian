#pragma once
#include "Game/System/NerveExecutor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class Meister;
class JMapInfoIter;
class DemoObj;
class MeisterDemoGrandStarReturn : public NerveExecutor {
public:
    MeisterDemoGrandStarReturn(Meister*, const JMapInfoIter&);
    virtual ~MeisterDemoGrandStarReturn();
    void start();
    void registerDemo(const char*, const char*, const JMapInfoIter&);
    Meister* mHost;
};
