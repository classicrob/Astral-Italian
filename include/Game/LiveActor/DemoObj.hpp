#pragma once

#include "Game/LiveActor/LiveActor.hpp"

/// Plain demo model actor (0x94 bytes, ctor 0x8023C460).
class DemoObj : public LiveActor {
public:
    DemoObj(const char* pName, MtxPtr pMtx);

    virtual ~DemoObj();
    virtual void init(const JMapInfoIter&);
    virtual void calcAndSetBaseMtx();

    /* 0x90 */ MtxPtr mMtx;
};
