#pragma once

#include <revolution/types.h>

class GhostPacketData;

class GhostPacketReader {
public:
    GhostPacketReader();
    void setSource(const void*, u32);
    bool read(GhostPacketData*);

    const u8* mSource;
    const u8* mCurrent;
    u32 mLength;
};

namespace MR {
    GhostPacketReader* createGhostPacketReader(const char*);
}
