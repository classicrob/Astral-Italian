#pragma once
#include "Game/MapObj/MapObjActor.hpp"
class ActorCameraInfo;
class AncientCage : public MapObjActor {
public:
    AncientCage(const char*);
    virtual ~AncientCage();
    virtual void init(const JMapInfoIter&);
    virtual void kill();
    virtual void initCaseUseSwitchA(const MapObjActorInitInfo&);
    virtual void initCaseNoUseSwitchA(const MapObjActorInitInfo&);
    virtual void initCaseUseSwitchB(const MapObjActorInitInfo&);
    virtual void initCaseNoUseSwitchB(const MapObjActorInitInfo&);
    void exeStartDemo();
    void exeOpen();
    void startOpenDemo();
    ActorCameraInfo* mCameraInfo;
};
