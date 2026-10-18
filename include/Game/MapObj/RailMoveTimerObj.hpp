#pragma once
#include "Game/MapObj/MapObjActor.hpp"
class RailMoveTimerObj : public MapObjActor {
public:
    RailMoveTimerObj(const char*);
    virtual ~RailMoveTimerObj();
    virtual void init(const JMapInfoIter&);
    void exeAppear();
    void exeMove();
    void exeDisappear();
    TVec3f mClippingCenter;
};
