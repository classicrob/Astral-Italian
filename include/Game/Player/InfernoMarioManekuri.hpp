#pragma once
#include "Game/Player/InfernoMarioStateBase.hpp"
class InfernoMarioManekuri : public InfernoMarioStateBase {
public:
    InfernoMarioManekuri(InfernoMario*);
    u8 _10[0x30];
};
