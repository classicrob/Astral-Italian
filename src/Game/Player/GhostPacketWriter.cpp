#include "Game/Player/GhostPacketWriter.hpp"
#include "Game/Player/GhostPacketWire.hpp"
#include "Game/Player/GhostPacketSyncData.hpp"
#include <cstring>

#pragma auto_inline off

namespace {
    u32 sFrameIndex;
}

GhostPacketWriter::GhostPacketWriter() {
    mSize = 0;
    mCurrentOffset = 0;
    u32 header = 0;
    write(reinterpret_cast<const u8*>(&header), 4);
    mCurrentOffset = 4;
}

void GhostPacketWriter::write(const GhostPacketSyncData* data, u32 flags, u32* frameIndex) {
    reset();
    if (flags & 1) write(data->mPackedPosition);
    if (flags & 0x4000) {
        GhostPacketWire::writeF32(*this, data->mPosition.x);
        GhostPacketWire::writeF32(*this, data->mPosition.y);
        GhostPacketWire::writeF32(*this, data->mPosition.z);
    }
    if (flags & 0x800) write(data->mVelocity);
    if (flags & 0x400) write(data->mScale);
    if (flags & 2) {
        s8 value = data->mRotation.x;
        write(reinterpret_cast<const u8*>(&value), 1);
    }
    if (flags & 4) {
        s8 value = data->mRotation.y;
        write(reinterpret_cast<const u8*>(&value), 1);
    }
    if (flags & 8) {
        s8 value = data->mRotation.z;
        write(reinterpret_cast<const u8*>(&value), 1);
    }
    if (flags & 0x10) {
        const char* name = data->mActionName;
        write(reinterpret_cast<const u8*>(name), strlen(name) + 1);
    }
    if (flags & 0x2000) {
        u32 value = data->mActionHash;
        GhostPacketWire::writeU32(*this, value);
    }
    if (flags & 0x20) {
        s16 value = data->mBckFrame;
        GhostPacketWire::writeS16(*this, value);
    }
    if (flags & 0x1000) {
        s8 value = data->mBckRate;
        write(reinterpret_cast<const u8*>(&value), 1);
    }
    if (flags & 0x40) {
        s8 value = data->mTrackWeights[0];
        write(reinterpret_cast<const u8*>(&value), 1);
    }
    if (flags & 0x80) {
        s8 value = data->mTrackWeights[1];
        write(reinterpret_cast<const u8*>(&value), 1);
    }
    if (flags & 0x100) {
        s8 value = data->mTrackWeights[2];
        write(reinterpret_cast<const u8*>(&value), 1);
    }
    if (flags & 0x200) {
        s8 value = data->mTrackWeights[3];
        write(reinterpret_cast<const u8*>(&value), 1);
    }
    finish(flags, frameIndex);
}

void GhostPacketWriter::reset() {
    mCurrentOffset = 0;
    u32 header = 0;
    write(reinterpret_cast<const u8*>(&header), 4);
    mCurrentOffset = 4;
}

void GhostPacketWriter::write(const u8* data, u32 size) {
    for (u32 index = 0; index < size; ++index) {
        mBuffer[mCurrentOffset] = *data++;
        ++mCurrentOffset;
    }
}

void GhostPacketWriter::write(const TVec3Sc& data) {
    s8 x = data.x;
    write(reinterpret_cast<const u8*>(&x), 1);
    s8 y = data.y;
    write(reinterpret_cast<const u8*>(&y), 1);
    s8 z = data.z;
    write(reinterpret_cast<const u8*>(&z), 1);
}

void GhostPacketWriter::write(const TVec3s& data) {
    s16 x = data.x;
    GhostPacketWire::writeS16(*this, x);
    s16 y = data.y;
    GhostPacketWire::writeS16(*this, y);
    s16 z = data.z;
    GhostPacketWire::writeS16(*this, z);
}

u32 GhostPacketWriter::finish(u32 flags, u32* frameIndex) {
    mSize = mCurrentOffset;
    u32 index = sFrameIndex;
    if (frameIndex != nullptr) {
        index = *frameIndex;
        ++*frameIndex;
    } else {
        ++sFrameIndex;
    }
    mCurrentOffset = 0;
    u8 indexByte = index;
    write(&indexByte, 1);
    u8 sizeByte = mSize;
    write(&sizeByte, 1);
    u16 mask = flags;
    GhostPacketWire::writeU16(*this, mask);
    return index;
}
