#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class PoleObstacleParts : public LiveActor {
public:
    PoleObstacleParts(const char*, s32, const char*);
    virtual ~PoleObstacleParts();
    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void calcAndSetBaseMtx();
    s32 mPartsType;
    const char* mModelName;
    bool mAlignToGravity;
};
