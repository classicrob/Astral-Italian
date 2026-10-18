#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class ActorCameraInfo;
class MiniPlayerRocket;

/// Rocket, camera target and camera info of the world map (0x68 bytes, WorldMapModel::mCamera).
class WorldMapCameraHolder {
public:
    WorldMapCameraHolder();

    void init(s32 current);
    void setNext(s32 next);
    bool tryApplyNext();

    f32 mAccel;                      // 0x00
    f32 mMaxSpeed;                   // 0x04
    f32 mFriction;                   // 0x08
    TVec3f mVelocity;                // 0x0C
    TVec3f mTargetA;                 // 0x18
    TVec3f mTargetB;                 // 0x24
    TVec3f mStartOffset;             // 0x30
    s32 mCurrent;                    // 0x3C
    s32 mPathFrom;                   // 0x40
    s32 mPathStep;                   // 0x44
    s32 mNext;                       // 0x48
    s32 mMoveFrame;                  // 0x4C
    MiniPlayerRocket* mRocket;       // 0x50
    LiveActor* mCameraTarget;        // 0x54
    ActorCameraInfo* mCameraInfo;    // 0x58
    s32 mCameraId;                   // 0x5C
    bool mMovingLeft;                // 0x60
    bool mHasNext;                   // 0x61
    s32 mPrevious;                   // 0x64
};
