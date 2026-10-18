#pragma once

#include "Game/MapObj/MapObjActor.hpp"

class BigRepeatWarpLift : public MapObjActor {
public:
    BigRepeatWarpLift(const char*);

    virtual ~BigRepeatWarpLift();
    virtual void init(const JMapInfoIter&);

    void exeAppear();
    void exeMove();
    void exeSign();
    void exeDisappear();

    /* 0xD0 */ TVec3f mRailClipCenter;
};
