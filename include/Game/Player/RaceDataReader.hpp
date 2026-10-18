#pragma once

#include <JSystem/JGeometry/TMatrix.hpp>

class LiveActor;
class GhostPacketReader;
class XanimePlayer;
class XanimeResourceTable;

class RaceDataReader {
public:
    RaceDataReader(LiveActor*, const char*, const XanimeResourceTable*);
    s32 receivePacket();
    void init(const XanimeResourceTable*);
    void startActionName(const char*);

    LiveActor* mHost;
    GhostPacketReader* mPacketReader;
    TPos3f mBaseMtx;
    XanimePlayer* mXanimePlayer;
    XanimePlayer* mGhostXanimePlayer;
    bool mHasAnimation;
};
