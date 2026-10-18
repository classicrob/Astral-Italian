#pragma once

#include "Game/Screen/LayoutActor.hpp"

class TimeUpLayout : public LayoutActor {
public:
    TimeUpLayout();
    virtual ~TimeUpLayout();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    void disappear();
    void exeAppear();
    void exeEnd();
};
