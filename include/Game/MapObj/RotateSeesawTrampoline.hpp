#pragma once
#include "Game/MapObj/MapObjActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class RotateSeesawTrampoline : public MapObjActor {
public:
    RotateSeesawTrampoline(const char*);
    virtual ~RotateSeesawTrampoline();
    virtual void init(const JMapInfoIter&);
    void exeWait();
    void exeMove();
    TPos3f mEffectMtx;
};
