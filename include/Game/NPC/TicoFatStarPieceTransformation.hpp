#pragma once
#include "Game/LiveActor/ActorStateBase.hpp"
class TicoFatStarPiece;
class ActorCameraInfo;
class JMapInfoIter;
class TicoFatStarPieceState : public ActorStateBase<TicoFatStarPiece> {
public:
    virtual ~TicoFatStarPieceState();
};
class TicoFatStarPieceTransformation : public TicoFatStarPieceState {
public:
    TicoFatStarPieceTransformation(TicoFatStarPiece*, const JMapInfoIter&);
    virtual ~TicoFatStarPieceTransformation();
    virtual void appear();
    ActorCameraInfo* mCameraInfo;
    u8 _14[0x18];
};
