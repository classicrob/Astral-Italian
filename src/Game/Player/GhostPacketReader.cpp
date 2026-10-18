#include "Game/Player/GhostPacketReader.hpp"
#include "Game/Player/GhostPacket.hpp"
#include "Game/Util/SceneUtil.hpp"

GhostPacketReader::GhostPacketReader() : mSource(nullptr), mCurrent(nullptr), mLength(0) {}

void GhostPacketReader::setSource(const void* data, u32 length) {
    mSource = static_cast<const u8*>(data);
    mLength = length;
    mCurrent = static_cast<const u8*>(data);
}

bool GhostPacketReader::read(GhostPacketData* output) {
    if (mCurrent > mSource + mLength) return false;
    GhostPacket packet(mCurrent);
    u8 length = packet.read(output);
    if (length == 0) return false;
    mCurrent += length;
    return true;
}

namespace MR {
    GhostPacketReader* createGhostPacketReader(const char* name) {
        const void* data = nullptr;
        u32 length = 0;
        getGhostData(&data, &length, name, 0);
        if (data == nullptr) return nullptr;
        GhostPacketReader* reader = new GhostPacketReader;
        reader->setSource(data, length);
        return reader;
    }
}
