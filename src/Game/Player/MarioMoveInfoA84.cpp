#include "Game/Player/MarioMoveInfoA84.hpp"

namespace {
    TVec3f sDirections[] = {
        TVec3f(0.0f, 0.0f, 0.0f),
        TVec3f(1.0f, 0.0f, 0.0f),
        TVec3f(-1.0f, 0.0f, 0.0f),
        TVec3f(0.0f, 1.0f, 0.0f),
        TVec3f(0.0f, -1.0f, 0.0f),
        TVec3f(0.0f, 0.0f, 1.0f),
        TVec3f(0.0f, 0.0f, -1.0f)
    };
}

MarioMoveInfoA84::MarioMoveInfoA84() : _0(0.0f), _C(0.0f), _10(0.0f), _14(0.0f), _18(0) {
    _1C.identity();
}
