#pragma once

#include "Game/MapObj/MapPartsSeesaw2AxisRotator.hpp"

// SMG2-only (0xF0 bytes).
class MapPartsSeesaw2AxisRollerRotator : public MapPartsSeesaw2AxisRotator {
public:
    MapPartsSeesaw2AxisRollerRotator(LiveActor*, const char*, f32);

    /* 0xE4 */ u8 _E4[0xF0 - 0xE4];
};
