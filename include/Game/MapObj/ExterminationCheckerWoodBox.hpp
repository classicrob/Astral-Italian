#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class WoodBox;
class ExterminationCheckerWoodBox : public LiveActor {
public:
    ExterminationCheckerWoodBox(const char*);
    virtual ~ExterminationCheckerWoodBox();
    virtual void init(const JMapInfoIter&);
    void exeMove();
    bool checkAllDestroyed();
    WoodBox** mBoxes;
    s32 mBoxCount;
};
