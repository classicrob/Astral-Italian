#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class WhiteSnowTree : public LiveActor {
public:
    WhiteSnowTree(const char*);
    virtual ~WhiteSnowTree();
    virtual void init(const JMapInfoIter&);
};
