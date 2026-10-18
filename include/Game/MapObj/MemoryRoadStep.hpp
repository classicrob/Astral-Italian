#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class MemoryRoadStep : public LiveActor {
public:
    MemoryRoadStep(const char*);
    virtual ~MemoryRoadStep();
    virtual void init(const JMapInfoIter&);
    void exeWait();
    bool tryFlashAnim();
};
