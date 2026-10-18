#pragma once

#include "Game/LiveActor/LiveActor.hpp"

class KoopaJrCastleGate : public LiveActor {
public:
    KoopaJrCastleGate(const char*);

    virtual ~KoopaJrCastleGate();
    virtual void init(const JMapInfoIter&) override;

    void exeWait();
    void exeMove();
    void exeMoveFast();
};
