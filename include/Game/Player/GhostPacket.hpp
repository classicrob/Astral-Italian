#pragma once

#include <revolution/types.h>

class GhostPacketData;

class GhostPacket {
public:
    GhostPacket(const void*);
    void read(u8*, u32);
    void read(char**);
    u8 read(GhostPacketData*);

    /* 0x0 */ const u8* mDataPtr;
    /* 0x4 */ u32 _4;
    /* 0x8 */ u32 mCurOffs;
};
