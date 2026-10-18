#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class GliderCometMedalCage : public LiveActor {
public:
    GliderCometMedalCage(const char*);
    virtual ~GliderCometMedalCage();
    virtual void init(const JMapInfoIter&);
    void exeWait();
    void exeOpen();
    s32 _90;
};
