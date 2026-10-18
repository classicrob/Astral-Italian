#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class MapPartsRailMover;
class MarioLauncherTarget : public LiveActor {
public:
    MarioLauncherTarget(const char*);
    virtual ~MarioLauncherTarget();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    void exeHit();
    MapPartsRailMover* mRailMover;
    TVec3f mClippingCenter;
};
