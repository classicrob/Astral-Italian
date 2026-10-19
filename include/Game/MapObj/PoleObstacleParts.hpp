#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class PoleObstacleParts : public LiveActor {
public:
    PoleObstacleParts(const char*, const LiveActor*, const char*);
    virtual ~PoleObstacleParts();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void calcAndSetBaseMtx();
    const LiveActor* mPlacementActor;
    const char* mModelName;
    bool mAlignToGravity;
};
