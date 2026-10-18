#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"

class GameSequence;

class GameSequenceSystemStart : public ActorStateBase< GameSequence > {
public:
    GameSequenceSystemStart(GameSequence* pHost);

    virtual void init();
    virtual void appear();
    virtual ~GameSequenceSystemStart();
};
