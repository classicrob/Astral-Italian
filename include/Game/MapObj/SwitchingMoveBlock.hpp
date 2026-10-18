#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class SwitchingMoveBlock : public LiveActor {
public:
    SwitchingMoveBlock(const char*);

    virtual ~SwitchingMoveBlock();
    virtual void init(const JMapInfoIter&) override;
    virtual void control() override;
    void listenSwitchOnA();
    void listenSwitchOffA();
    void exeMove();

    /* 0x90 */ void* mMoveFunction;
};
