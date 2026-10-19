#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class MoveBlockFunction;

class SwitchingMoveBlock : public LiveActor {
public:
    SwitchingMoveBlock(const char*);

    virtual ~SwitchingMoveBlock();
    virtual void init(const JMapInfoIter&) override;
    virtual void control() override;
    virtual void calcAndSetBaseMtx();
    void listenSwitchOnA();
    void listenSwitchOffA();
    void exeMove();

    /* 0x90 */ MoveBlockFunction* mMoveFunction;
};
