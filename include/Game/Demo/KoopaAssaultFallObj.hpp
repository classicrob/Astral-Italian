#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ModelObj;

class KoopaAssaultFallObj : public LiveActor {
public:
    KoopaAssaultFallObj(const char*);

    virtual ~KoopaAssaultFallObj();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void makeActorDead();

    void exeFall();
    void exeBreak();

    /* 0x90 */ TVec3f mBasePos;
    /* 0x9C */ ModelObj* mModel;
};
