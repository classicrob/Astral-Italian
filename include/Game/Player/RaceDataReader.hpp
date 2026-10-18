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
    void startActionHash(u32);
    void setBckFrame(f32);
    void setTrackWeight(u32, f32);
    void setBckRate(f32);
    void setMtxTR();

    LiveActor* mHost;
    GhostPacketReader* mPacketReader;
    TPos3f mBaseMtx;
    XanimePlayer* mXanimePlayer;
    XanimePlayer* mGhostXanimePlayer;
    bool mHasAnimation;
};
