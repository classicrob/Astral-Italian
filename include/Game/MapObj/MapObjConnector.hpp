#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

struct MapObjConnectorAttachment {
    bool mAttached;
    MtxPtr mBaseMtx;
};

class MapObjConnector {
public:
    MapObjConnector(LiveActor*);

    void attachToActor(LiveActor*);
    void detach();
    bool attachToUnder();
    bool attachToBack();
    bool attach(const TVec3f&);
    void connect();
    void connect(LiveActor*);

    /* 0x0 */ LiveActor* mHost;
    /* 0x4 */ MapObjConnectorAttachment* mAttachment;
    /* 0x8 */ TMtx34f _8;
};
