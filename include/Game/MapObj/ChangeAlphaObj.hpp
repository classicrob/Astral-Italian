#pragma once
#include "Game/MapObj/MapObjActor.hpp"
class ChangeAlphaObj : public MapObjActor {
public:
    ChangeAlphaObj(const char*);
    virtual ~ChangeAlphaObj();
    virtual void init(const JMapInfoIter&);
    void exeWait();
    void exeAlpha();
    bool isNeedForKill();
    ModelObj* mAlphaModel;
};
