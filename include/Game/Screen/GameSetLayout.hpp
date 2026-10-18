#pragma once
#include "Game/Screen/LayoutActor.hpp"

class GameSetLayout : public LayoutActor {
public:
    GameSetLayout();
    virtual ~GameSetLayout();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    void disappear();
    void exeAppear();
    void exeEnd();
};
