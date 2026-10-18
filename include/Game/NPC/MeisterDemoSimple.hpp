#pragma once
#include "Game/System/NerveExecutor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class Meister;
class JMapInfoIter;
class DemoObj;
class MeisterDemoSimple : public NerveExecutor {
public:
    MeisterDemoSimple(Meister*, const JMapInfoIter&);
    virtual ~MeisterDemoSimple();
    void start();
    void registerDemo(const char*, const char*, const JMapInfoIter&);
    Meister* mHost;
    u32 _C;
};
