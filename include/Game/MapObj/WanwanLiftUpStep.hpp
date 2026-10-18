#pragma once
#include "Game/MapObj/RailMoveObj.hpp"
class WanwanLiftUpStep : public RailMoveObj {
public:
    WanwanLiftUpStep(const char*);
    virtual ~WanwanLiftUpStep();
    virtual void init(const JMapInfoIter&);
    virtual void initCaseUseSwitchB(const MapObjActorInitInfo&);
    void listenSwitchOnB();
    void listenSwitchOffB();
    s32 mEffectType;
};
