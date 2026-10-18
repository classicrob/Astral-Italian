#pragma once
#include "Game/MapObj/MapObjActor.hpp"
class RailMoveDisappearStep : public MapObjActor {
public:
    RailMoveDisappearStep(const char*);
    virtual ~RailMoveDisappearStep();
    virtual void init(const JMapInfoIter&);
    void exeSign();
    void exeMove();
    void exeBreak();
    TVec3f mRailClipPosition;
};
