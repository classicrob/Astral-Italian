#pragma once
#include "Game/LiveActor/LiveActor.hpp"

class StarPieceSpotEruption : public LiveActor {
public:
    StarPieceSpotEruption(const char*);
    virtual ~StarPieceSpotEruption();
    virtual void init(const JMapInfoIter&);
    void exeRest();
    void emit();
    f32 mEmitRange;
    f32 mEmitSpeed;
};
