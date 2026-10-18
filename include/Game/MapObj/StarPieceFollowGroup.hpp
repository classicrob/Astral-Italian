#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class StarPiece;

class StarPieceFollowGroup : public LiveActor {
public:
    StarPieceFollowGroup(const char*);

    virtual void init(const JMapInfoIter& rIter);

    bool beginFollowPieces();
    void placementPiece(s32);
    void updateAxises(const TVec3f&, const TVec3f&, const TVec3f&, f32);
    void followPieces();
    void allKillPieces();
    void onSwitchA();
    void exeFollowToPlayer();
    void exeFollowEnd();

    /* 0x90 */ StarPiece** mPieces;
    /* 0x94 */ TVec3f* _90;
    /* 0x98 */ s32 mNumPiecesTotal;
    /* 0x9C */ s32 mNumPiecesUse;
    /* 0xA0 */ f32 mRadius;
    /* 0xA4 */ f32 mUnused;
    /* 0xA8 */ f32 mVelocityScale;
    /* 0xAC */ f32 mAngle;
    /* 0xB0 */ s32 mTimeLimit;
    /* 0xB4 */ TVec3f mLocalXAxis;
    /* 0xC0 */ TVec3f mLocalZAxis;
    /* 0xCC */ TVec3f mLocalYAxis;
};
