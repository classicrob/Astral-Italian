#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class MapPartsRailMover;
class RailMoveNormalLift : public LiveActor {
public:
    RailMoveNormalLift(const char*);
    virtual ~RailMoveNormalLift();
    virtual void init(const JMapInfoIter&);
    void exeMove();
    MapPartsRailMover* mRailMover;
    TVec3f mClippingCenter;
};
