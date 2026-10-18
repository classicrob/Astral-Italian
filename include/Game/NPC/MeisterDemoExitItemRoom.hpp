#pragma once
#include "Game/System/NerveExecutor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class Meister;
class JMapInfoIter;
class DemoObj;
class MeisterDemoExitItemRoom : public NerveExecutor {
public:
    MeisterDemoExitItemRoom(Meister*, const JMapInfoIter&);
    virtual ~MeisterDemoExitItemRoom();
    void start();
    void exeWait();
    Meister* mHost;
    bool mRepositionHost;
};
