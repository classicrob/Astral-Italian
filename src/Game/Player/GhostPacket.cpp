#include "Game/Player/GhostPacket.hpp"
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

// NONMATCHING: local stack placement and redundant initial flags reload.
u8 GhostPacket::read(GhostPacketData* output) {
    u8 frameIndex;
    u8 length;
    u16 flags;
    mCurOffs = 0;
    read(&frameIndex, 1);
    read(&length, 1);
    read(reinterpret_cast<u8*>(&flags), 2);
    output->mUpdateFlags = flags;
    if (flags & 1) {
        TVec3s position;
        read(reinterpret_cast<u8*>(&position.x), 2);
        read(reinterpret_cast<u8*>(&position.y), 2);
        read(reinterpret_cast<u8*>(&position.z), 2);
        MR::convToFloat(position, -2, &output->mPosition);
    } else if (flags & 0x4000) {
        read(reinterpret_cast<u8*>(&output->mPosition.x), 4);
        read(reinterpret_cast<u8*>(&output->mPosition.y), 4);
        read(reinterpret_cast<u8*>(&output->mPosition.z), 4);
    }
    if (flags & 0x800) {
        TVec3Sc velocity;
        read(reinterpret_cast<u8*>(&velocity.x), 1);
        read(reinterpret_cast<u8*>(&velocity.y), 1);
        read(reinterpret_cast<u8*>(&velocity.z), 1);
        MR::convToFloat(velocity, 0, &output->mVelocity);
    }
    if (flags & 0x400) {
        TVec3Sc scale;
        read(reinterpret_cast<u8*>(&scale.x), 1);
        read(reinterpret_cast<u8*>(&scale.y), 1);
        read(reinterpret_cast<u8*>(&scale.z), 1);
        MR::convToFloat(scale, 3, &output->mScale);
    }
    if (flags & 2) {
        s8 value;
        read(reinterpret_cast<u8*>(&value), 1);
        MR::convToFloatRotation(&output->mRotation.x, value);
    }
    if (flags & 4) {
        s8 value;
        read(reinterpret_cast<u8*>(&value), 1);
        MR::convToFloatRotation(&output->mRotation.y, value);
    }
    if (flags & 8) {
        s8 value;
        read(reinterpret_cast<u8*>(&value), 1);
        MR::convToFloatRotation(&output->mRotation.z, value);
    }
    if (flags & 0x10) read(&output->mActionName);
    if (flags & 0x2000) read(reinterpret_cast<u8*>(&output->mActionHash), 4);
    if (flags & 0x20) {
        s16 value;
        read(reinterpret_cast<u8*>(&value), 2);
        MR::convToFloat(value, 5, &output->mBckFrame);
    }
    if (flags & 0x1000) {
        s8 value;
        read(reinterpret_cast<u8*>(&value), 1);
        MR::convToFloatBckRate(&output->mBckRate, value);
    }
    for (u32 index = 0; index < 4; ++index) {
        if (flags & (0x40 << index)) {
            s8 value;
            read(reinterpret_cast<u8*>(&value), 1);
            MR::convToFloatTrackWeight(&output->mTrackWeights[index], value);
        }
    }
    return length;
}
