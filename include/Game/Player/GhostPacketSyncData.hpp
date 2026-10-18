#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class LiveActor;

class GhostPacketSyncData {
public:
    GhostPacketSyncData();
    void syncWithPlayer();
    void syncWithActor(const LiveActor*, bool);

    TVec3f mPosition;
    TVec3s mPackedPosition;
    TVec3Sc mRotation;
    TVec3Sc mScale;
    TVec3Sc mVelocity;
    const char* mActionName;
    u32 mActionHash;
    bool mUsesActionHash;
    bool mPositionOverflow;
    s16 mBckFrame;
    s8 mTrackWeights[4];
    s8 mBckRate;
};
