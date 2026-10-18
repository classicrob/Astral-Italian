#pragma once
#include "Game/LiveActor/LiveActor.hpp"
class StarPieceTracking;
class StarPieceTrackingGenerator : public LiveActor {
public:
    void calcStarPieceDirection();
    s32 mPieceCount; // 0x90
    StarPieceTracking* mPieces;
    f32 mSpeed;
};
