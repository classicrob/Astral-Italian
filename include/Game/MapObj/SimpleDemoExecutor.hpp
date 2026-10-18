#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class ActorCameraInfo;
class JMapLinkInfo;
class SimpleDemoExecutor : public LiveActor {
public:
    SimpleDemoExecutor(const char*);
    virtual ~SimpleDemoExecutor();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    void exeWait();
    void exeDemo();
    LiveActor* mTarget;
    JMapLinkInfo* mLinkInfo;
    char* mDemoName;
    s32 mCameraFrames;
    s32 mSwitchFrame;
    ActorCameraInfo* mCameraInfo;
};
