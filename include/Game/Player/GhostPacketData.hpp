#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class GhostPacketData {
public:
    GhostPacketData();
    bool testPosition() const;
    bool testTRMtx() const;
    bool testRotationX() const;
    bool testRotationY() const;
    bool testRotationZ() const;
    bool testScale() const;
    bool testVelocity() const;
    bool testActionName() const;
    bool testActionHash() const;
    bool testBckFrame() const;
    bool testTrackWeight(u32) const;
    bool testTrackWeight0() const;
    bool testTrackWeight1() const;
    bool testTrackWeight2() const;
    bool testTrackWeight3() const;
    bool testBckRate() const;

    TVec3f mPosition;
    TVec3f mRotation;
    TVec3f mScale;
    TVec3f mVelocity;
    char* mActionName;
    u32 mActionHash;
    f32 mBckFrame;
    f32 mTrackWeights[4];
    f32 mBckRate;
    u32 mUpdateFlags;
};
