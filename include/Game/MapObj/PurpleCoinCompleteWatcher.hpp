#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class PurpleCoinCompleteWatcher : public LiveActor {
public:
    PurpleCoinCompleteWatcher(const char*);
    virtual ~PurpleCoinCompleteWatcher();
    virtual void init(const JMapInfoIter&);
    void exeWait();
};
