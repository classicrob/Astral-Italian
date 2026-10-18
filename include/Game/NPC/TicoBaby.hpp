#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class TicoBaby : public LiveActor {
public:
    TicoBaby(const char*);
    virtual ~TicoBaby();
    virtual void init(const JMapInfoIter&);
    virtual void kill();
    void exeWait();
    void exeDemo();
};
