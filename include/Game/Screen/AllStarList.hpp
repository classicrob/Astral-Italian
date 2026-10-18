#pragma once

#include "Game/Screen/LayoutActor.hpp"

class AllStarList : public LayoutActor {
public:
    AllStarList(const char*, bool);
    virtual ~AllStarList();
    virtual void init(const JMapInfoIter&);
    virtual void appear();
    virtual void control();

    void requestMovementOn();

    void* _2C;
    void* _30;
    void* _34;
    s32 _38;
    s32 mResult;
    bool _40;
};
