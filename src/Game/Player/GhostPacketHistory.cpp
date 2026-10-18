#include "Game/Player/GhostPacketHistory.hpp"
#include "Game/Player/GhostPacket.hpp"
#include "Game/Player/GhostPacketSyncData.hpp"
#include "Game/Player/GhostPacketWriter.hpp"
#include "Game/Player/GhostPlayer.hpp"
#include <cstring>

GhostPacketHistory::GhostPacketHistory(u32 frames, u32 flags) {
    mPackets = nullptr;
    mPacketSize = 0;
    mFrameCount = 0;
    mCurrentFrame = 0;
    mUpdateFlags = flags;
    mSyncState = 0;
    mWrapCount = 0;
    mPacketSize = MR::fn_8037FE20(flags);
    if (mUpdateFlags & 0x10) mPacketSize += 0x40;
    mPackets = new u8*[frames];
    for (u32 index = 0; index < frames; ++index) mPackets[index] = new u8[mPacketSize];
    mFrameCount = frames;
    mCurrentFrame = 0;
}

void GhostPacketHistory::update() {
    GhostPacketSyncData data;
    data.syncWithPlayer();
    GhostPacketWriter writer;
    u32 flags = mUpdateFlags;
    if (!data.mUsesActionHash) flags &= ~0x2000;
    writer.write(&data, flags, &mSyncState);
    memcpy(mPackets[mCurrentFrame], writer.mBuffer, writer.mSize);
    ++mCurrentFrame;
    if (mCurrentFrame >= mFrameCount) {
        mCurrentFrame = 0;
        ++mWrapCount;
    }
}

bool GhostPacketHistory::read(s32 framesAgo, GhostPacketData* output) {
    if (framesAgo < 0 || static_cast<u32>(framesAgo) >= mFrameCount) return false;
    if (mWrapCount == 0 && static_cast<u32>(framesAgo) >= mCurrentFrame) return false;
    s32 index = mCurrentFrame - (framesAgo + 1);
    if (index < 0) index += mFrameCount;
    GhostPacket packet(mPackets[index]);
    packet.read(output);
    return true;
}
