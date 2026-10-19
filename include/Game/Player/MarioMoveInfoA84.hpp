#pragma once

#include <JSystem/JGeometry/TVec.hpp>
#include <JSystem/JGeometry/TMatrix.hpp>

class MarioMoveInfoA84 {
public:
    MarioMoveInfoA84();

    /* 0x00 */ TVec3f _0;
    /* 0x0C */ f32 _C;
    /* 0x10 */ f32 _10;
    /* 0x14 */ f32 _14;
    /* 0x18 */ u32 _18;
    /* 0x1C */ TMtx34f _1C;
};
