#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class CollisionObj : public LiveActor {
public:
    CollisionObj(LiveActor* pHost, const char* pName, const char* pArcName, bool noSensor, MtxPtr pBaseMtx, int drawType);

    virtual void init(const JMapInfoIter&);
    virtual void movement();
    virtual MtxPtr getBaseMtx() const;

    /* 0x90 */ MtxPtr mBaseMtx;
};
