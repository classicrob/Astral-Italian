#pragma once
#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/J3DGraphBase/J3DStruct.hpp>
// Provisional recovered name, supported by sound resource StarPieceScatter and rocket damage callers.
class StarPieceScatter : public LiveActor {
public:
    StarPieceScatter(const char*);
    virtual ~StarPieceScatter();
    virtual void init(const JMapInfoIter&);
    void scatter(const TVec3f& position,const TVec3f& up,f32 randomMagnitude,f32 upSpeed);
    void exeScatter();
    J3DGXColor mColor; //90
    TVec3f mUp; //94
};
