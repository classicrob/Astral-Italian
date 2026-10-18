#pragma once
#include "Game/LiveActor/ActorStateBase.hpp"
#include <JSystem/JGeometry/TVec.hpp>
class TicoFatStarPiece;
class ActorCameraInfo;
class JMapInfoIter;
class TicoFatStarPieceState : public ActorStateBase<TicoFatStarPiece> {
public:
    TicoFatStarPieceState(const char* name, TicoFatStarPiece* host) : ActorStateBase<TicoFatStarPiece>(name, host) {}
    virtual ~TicoFatStarPieceState();
};
class TicoFatStarPieceTransformation : public TicoFatStarPieceState {
public:
    TicoFatStarPieceTransformation(TicoFatStarPiece*, const JMapInfoIter&);
    virtual ~TicoFatStarPieceTransformation();
    void exeWaitDemo();
    void exeWaitTalk();
    void exeFullness();
    void exeTransform();
    void exeFly();
    void exeCreateRoute();
    ActorCameraInfo* mCameraInfo;
    TVec3f mStartPos;
    TVec3f mEndPos;
};
