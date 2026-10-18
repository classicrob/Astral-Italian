#pragma once

#include <revolution/types.h>

class GhostPacketData;

class GhostPacketHistory {
public:
    GhostPacketHistory(u32, u32);
    void update();
    bool read(s32, GhostPacketData*);

    u8** mPackets;
    u32 mPacketSize;
    u32 mFrameCount;
    u32 mCurrentFrame;
    u32 mUpdateFlags;
    u32 mSyncState;
    u32 mWrapCount;
};
