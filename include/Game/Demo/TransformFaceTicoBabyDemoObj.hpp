#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class DemoObj;

class TransformFaceTicoBabyDemoObj : public LiveActor {
public:
    TransformFaceTicoBabyDemoObj(const char*);

    virtual ~TransformFaceTicoBabyDemoObj();
    virtual void init(const JMapInfoIter&);
    virtual void kill();

    /* 0x90 */ DemoObj* mDemoObj;
};
