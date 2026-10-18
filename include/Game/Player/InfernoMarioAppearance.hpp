#pragma once
#include "Game/LiveActor/ActorStateBase.hpp"
class InfernoMario;
class InfernoMarioAppearance : public ActorStateBase<InfernoMario> {
public:
    InfernoMarioAppearance(InfernoMario*);
    u8 _10[0x18];
};
