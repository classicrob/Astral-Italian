#pragma once
#include "Game/Screen/LayoutActor.hpp"
class StartCountdownLayout : public LayoutActor {
public:
    StartCountdownLayout();
    virtual ~StartCountdownLayout();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    bool isStart() const;
    void exeCountdown();
    void exeStart();
};
