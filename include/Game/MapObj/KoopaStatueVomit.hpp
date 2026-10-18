#pragma once
#include "Game/MapObj/MapObjActor.hpp"
class KoopaStatueVomit : public MapObjActor {
public:
    KoopaStatueVomit(const char*);
    virtual ~KoopaStatueVomit();
    virtual void init(const JMapInfoIter&);
    virtual void initCaseUseSwitchA(const MapObjActorInitInfo&);
    virtual void initCaseNoUseSwitchA(const MapObjActorInitInfo&);
    virtual void initCaseUseSwitchB(const MapObjActorInitInfo&);
    virtual void initCaseNoUseSwitchB(const MapObjActorInitInfo&);
    void exeWait();
    void exeRotate();
    bool mRotateRight;
};
