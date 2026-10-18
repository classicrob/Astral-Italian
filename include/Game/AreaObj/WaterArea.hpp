#pragma once

#include "Game/AreaObj/AreaObj.hpp"

class WaterArea : public AreaObj {
public:
    WaterArea(const char*);

    virtual void init(const JMapInfoIter&);
};

class WaterAreaMgr : public AreaObjMgr {
public:
    WaterAreaMgr(s32, const char*);
};
