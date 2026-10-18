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

    u8 _0[0x3C];
    s32 mCurrent;                    // 0x3C
    u8 _40[8];
    s32 mNext;                       // 0x48
    u8 _4C[4];
    MiniPlayerRocket* mRocket;       // 0x50
    LiveActor* mCameraTarget;        // 0x54
    ActorCameraInfo* mCameraInfo;    // 0x58
    s32 mCameraId;                   // 0x5C
    bool mIsChanging;                // 0x60
    bool mHasNext;                   // 0x61
    s32 mPrevious;                   // 0x64
};
