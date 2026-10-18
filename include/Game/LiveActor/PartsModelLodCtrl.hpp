#pragma once

#include "Game/LiveActor/PartsModel.hpp"

class LodCtrl;

// SMG2-only (Game/LiveActor/PartsModelLodCtrl.cpp, 0x8023CF60). PartsModel + LodCtrl.
class PartsModelLodCtrl : public PartsModel {
public:
    PartsModelLodCtrl(LiveActor* pHost, const char* pName, const char* pModelName, MtxPtr pMtx, s32 drawBufferType, bool);

    virtual ~PartsModelLodCtrl();
    virtual void init(const JMapInfoIter& rIter);
    virtual void control();
    virtual void calcAndSetBaseMtx();

    LodCtrl* getLodCtrl();
    void initLightCtrl();

    /* 0xA0 */ LodCtrl* mLodCtrl;
};
