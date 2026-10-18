#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ActorCameraInfo;

class MarioFaceAfterGrandStarTakeOffDemoObj : public LiveActor {
public:
    MarioFaceAfterGrandStarTakeOffDemoObj(const char*);

    virtual void init(const JMapInfoIter&);
    virtual ~MarioFaceAfterGrandStarTakeOffDemoObj();

    void exeDemo();

    /* 0x90 */ ActorCameraInfo* mCameraInfo;
    /* 0x94 */ LiveActor** mParts;
    /* 0x98 */ s32 mCount;
};
