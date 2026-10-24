#include "Game/Player/GhostPacket.hpp"
#include "Game/Player/GhostPacketWire.hpp"
#include "Game/Player/GhostPacketData.hpp"
#include "Game/Player/GhostPlayer.hpp"
#include <cstring>

GhostPacket::GhostPacket(const void* data) : mDataPtr(static_cast<const u8*>(data)), mCurOffs(0) {}

void GhostPacket::read(u8* output, u32 length) {
    for (u32 index = 0; index < length; ++index) {
        *output = mDataPtr[mCurOffs];
        ++output;
        ++mCurOffs;
    }
}

void GhostPacket::read(char** output) {
    char* string = const_cast<char*>(reinterpret_cast<const char*>(mDataPtr + mCurOffs));
    *output = string;
    s32 length = strlen(string) + 1;
    mCurOffs += length;
}

u8 GhostPacket::read(GhostPacketData* output) {
    struct {
        u8 frameIndex;
        u8 length;
        u16 flags;
    } header;
    mCurOffs = 0;
    read(&header.frameIndex, 1);
    read(&header.length, 1);
    GhostPacketWire::readU16(*this, header.flags);
    output->mUpdateFlags = header.flags;
    if (header.flags & 1) {
        TVec3s position;
        GhostPacketWire::readS16(*this, position.x);
        GhostPacketWire::readS16(*this, position.y);
        GhostPacketWire::readS16(*this, position.z);
        MR::convToFloat(position, -2, &output->mPosition);
    } else if (header.flags & 0x4000) {
        GhostPacketWire::readF32(*this, output->mPosition.x);
        GhostPacketWire::readF32(*this, output->mPosition.y);
        GhostPacketWire::readF32(*this, output->mPosition.z);
    }
    if (header.flags & 0x800) {
        TVec3Sc velocity;
        read(reinterpret_cast<u8*>(&velocity.x), 1);
        read(reinterpret_cast<u8*>(&velocity.y), 1);
        read(reinterpret_cast<u8*>(&velocity.z), 1);
        MR::convToFloat(velocity, 0, &output->mVelocity);
    }
    if (header.flags & 0x400) {
        TVec3Sc scale;
        read(reinterpret_cast<u8*>(&scale.x), 1);
        read(reinterpret_cast<u8*>(&scale.y), 1);
        read(reinterpret_cast<u8*>(&scale.z), 1);
        MR::convToFloat(scale, 3, &output->mScale);
    }
    if (header.flags & 2) {
        s8 value;
        read(reinterpret_cast<u8*>(&value), 1);
        MR::convToFloatRotation(&output->mRotation.x, value);
    }
    if (header.flags & 4) {
        s8 value;
        read(reinterpret_cast<u8*>(&value), 1);
        MR::convToFloatRotation(&output->mRotation.y, value);
    }
    if (header.flags & 8) {
        s8 value;
        read(reinterpret_cast<u8*>(&value), 1);
        MR::convToFloatRotation(&output->mRotation.z, value);
    }
    if (header.flags & 0x10) read(&output->mActionName);
    if (header.flags & 0x2000) GhostPacketWire::readU32(*this, output->mActionHash);
    if (header.flags & 0x20) {
        s16 value;
        GhostPacketWire::readS16(*this, value);
        MR::convToFloat(value, 5, &output->mBckFrame);
    }
    if (header.flags & 0x1000) {
        s8 value;
        read(reinterpret_cast<u8*>(&value), 1);
        MR::convToFloatBckRate(&output->mBckRate, value);
    }
    for (u32 index = 0; index < 4; ++index) {
        if (header.flags & (0x40 << index)) {
            s8 value;
            read(reinterpret_cast<u8*>(&value), 1);
            MR::convToFloatTrackWeight(&output->mTrackWeights[index], value);
        }
    }
    return header.length;
}
