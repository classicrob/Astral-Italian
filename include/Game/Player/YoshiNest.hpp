#pragma once

#include "Game/LiveActor/ModelObj.hpp"

class MapObjConnector;

class YoshiNest : public ModelObj {
public:
    YoshiNest(const char*, const char*, MtxPtr, s32, s32, s32, bool);
    virtual ~YoshiNest();
    virtual void calcAnim();
    MapObjConnector* mConnector;
    bool mConnected;
};
