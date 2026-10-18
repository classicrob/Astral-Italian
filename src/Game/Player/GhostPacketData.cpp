#include "Game/Player/GhostPacketData.hpp"

GhostPacketData::GhostPacketData() : mPosition(0.0f) {
    mRotation.set(0.0f, 0.0f, 0.0f);
    mScale.set(1.0f, 1.0f, 1.0f);
    mVelocity.set(0.0f, 0.0f, 0.0f);
    mActionName = nullptr;
    mActionHash = 0;
    mBckFrame = 0.0f;
    mBckRate = 1.0f;
    mUpdateFlags = 0;
    for (s32 index = 0; index < 4; ++index) mTrackWeights[index] = 0.0f;
}

bool GhostPacketData::testPosition() const { return (mUpdateFlags & 0x4001) != 0; }
bool GhostPacketData::testTRMtx() const { return (mUpdateFlags & 0xE) != 0; }
bool GhostPacketData::testRotationX() const { return (mUpdateFlags >> 1) & 1; }
bool GhostPacketData::testRotationY() const { return (mUpdateFlags >> 2) & 1; }
bool GhostPacketData::testRotationZ() const { return (mUpdateFlags >> 3) & 1; }
bool GhostPacketData::testScale() const { return (mUpdateFlags >> 10) & 1; }
bool GhostPacketData::testVelocity() const { return (mUpdateFlags >> 11) & 1; }
bool GhostPacketData::testActionName() const { return (mUpdateFlags >> 4) & 1; }
bool GhostPacketData::testActionHash() const { return (mUpdateFlags >> 13) & 1; }
bool GhostPacketData::testBckFrame() const { return (mUpdateFlags >> 5) & 1; }
bool GhostPacketData::testTrackWeight(u32 index) const { return (mUpdateFlags & (0x40 << index)) != 0; }
bool GhostPacketData::testTrackWeight0() const { return (mUpdateFlags >> 6) & 1; }
bool GhostPacketData::testTrackWeight1() const { return (mUpdateFlags >> 7) & 1; }
bool GhostPacketData::testTrackWeight2() const { return (mUpdateFlags >> 8) & 1; }
bool GhostPacketData::testTrackWeight3() const { return (mUpdateFlags >> 9) & 1; }
bool GhostPacketData::testBckRate() const { return (mUpdateFlags >> 12) & 1; }
