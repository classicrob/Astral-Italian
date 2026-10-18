#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class OldFortressCover : public LiveActor {
public:
    OldFortressCover(const char*);
    virtual ~OldFortressCover();
    virtual void init(const JMapInfoIter&);
    void exeOpen();
};
