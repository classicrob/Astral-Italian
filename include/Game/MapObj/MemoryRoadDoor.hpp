#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class MemoryRoadDoor : public LiveActor {
public:
    MemoryRoadDoor(const char*);
    virtual ~MemoryRoadDoor();
    virtual void init(const JMapInfoIter&);
    void exeOpen();
    void exeClose();
    bool tryFlashAnim();
};
