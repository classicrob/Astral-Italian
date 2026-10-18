#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class ProjmapEffectMtxSetter;
class GravityTimerChangeWall : public LiveActor {
public:
    GravityTimerChangeWall(const char*);
    virtual ~GravityTimerChangeWall();
    virtual void init(const JMapInfoIter&);
    void exeWait();
    void exeUpToDown();
    void exeDownToUp();
    bool tryChange();
    void listenSwitchOnA();
    s32 mWaitFrames;
    bool mIsChanging;
    bool mUseProjmap;
    ProjmapEffectMtxSetter* mProjmap;
};
