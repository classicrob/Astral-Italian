#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ActorCameraInfo;

class MarioFacePlanetTakeOffDemoObj : public LiveActor {
public:
    MarioFacePlanetTakeOffDemoObj(const char*);

    virtual void init(const JMapInfoIter&);
    virtual ~MarioFacePlanetTakeOffDemoObj();

    void exeWait();
    void exeDemo();

    /* 0x90 */ ActorCameraInfo* mCameraInfo;
    /* 0x94 */ LiveActor** mParts;
    /* 0x98 */ s32 mCount;
};
