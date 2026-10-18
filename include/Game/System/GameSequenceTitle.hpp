#pragma once

#include "Game/LiveActor/ActorStateBase.hpp"

class GameSequence;

class GameSequenceTitle : public ActorStateBase< GameSequence > {
public:
    GameSequenceTitle(GameSequence* pHost);

    virtual void init();
    virtual void appear();
    virtual ~GameSequenceTitle();
};
