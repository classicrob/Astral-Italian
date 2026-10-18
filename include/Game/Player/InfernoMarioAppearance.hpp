#pragma once
#include "Game/LiveActor/ActorStateBase.hpp"
#include <JSystem/JGeometry/TVec.hpp>
class InfernoMario;
class InfernoMarioAppearance : public ActorStateBase<InfernoMario> {
public:
    InfernoMarioAppearance(InfernoMario*);
    virtual ~InfernoMarioAppearance();
    virtual void appear();
    void exeJump();
    TVec3f mStartPos;
    TVec3f mUp;
};
