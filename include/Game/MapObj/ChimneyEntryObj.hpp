#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class ActorCameraInfo;
class ChimneyEntryObj : public LiveActor {
public:
    ChimneyEntryObj(const char*);
    virtual ~ChimneyEntryObj();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    void exeWarpStart();
    void exeWarp();
    void exeWarpEnd();
    void warpPlayer();
    TPos3f mEntranceMtx;
    LiveActor* mBindActor;
    TPos3f mPlayerMtx;
    TPos3f mPlayerStartMtx;
    TPos3f mPlayerTargetMtx;
    TPos3f mExitMtx;
    ActorCameraInfo* mCameraInfo;
};
