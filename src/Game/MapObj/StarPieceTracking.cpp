#pragma auto_inline off
#include "Game/MapObj/StarPieceTrackingGenerator.hpp"
#include "Game/MapObj/StarPieceTracking.hpp"
#include "Game/Util.hpp"
// NONMATCHING: negation temporary uses r4/r3 in the opposite order.
void StarPieceTrackingGenerator::calcStarPieceDirection() {
    TVec3f up(0.0f), side(0.0f), front(0.0f);
    front.set(*MR::getPlayerVelocity());
    if (MR::normalizeOrZero(&front)) { MR::calcFrontVec(&front, this); front = -front; }
    MR::calcUpVec(&up, this);
    PSVECCrossProduct(&up, &front, &side);
    if (MR::normalizeOrZero(&side)) MR::calcSideVec(&side, this);
    TVec3f oldFront(front);
    PSVECCrossProduct(&side, &up, &front);
    if (MR::normalizeOrZero(&side)) front.set(oldFront);
    TVec3f direction(front);
    if (mPieceCount != 1) {
        mPieces->appear(*MR::getSensorPos(this, "Body"), front, up, side, 180.0f, mSpeed, 100.0f, 2000.0f);
    } else {
        for (s32 i = 0; i < mPieceCount; ++i) {
            static_cast<StarPieceTrackingPiece*>(mPieces->getActor(i))->appearFromActor(this, direction, up, mSpeed, 200.0f, 2000.0f);
        }
    }
}
