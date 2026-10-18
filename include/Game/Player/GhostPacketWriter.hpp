#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class GhostPacketSyncData;

class GhostPacketWriter {
public:
    GhostPacketWriter();
    void write(const GhostPacketSyncData*, u32, u32*);
    void reset();
    void write(const u8*, u32);
    void write(const TVec3Sc&);
    void write(const TVec3s&);
    u32 finish(u32, u32*);

    u32 mSize;
    u8 mBuffer[0x100];
    u32 mCurrentOffset;
};
