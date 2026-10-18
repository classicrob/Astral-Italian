#include "Game/MapObj/RotateHammerPressure.hpp"
RotateHammerPressure::RotateHammerPressure(const char* name)
    : RotateMoveObj(name), _190(0.0f), _194(0.0f), _198(0.0f), _19C(0.0f), _1A0(0.0f), _1A4(0.0f), _1A8(false), _1A9(false), _1AC(0) {
    PSMTXIdentity(mMatrices[0]);
    PSMTXIdentity(mMatrices[1]);
    PSMTXIdentity(mMatrices[2]);
    PSMTXIdentity(mMatrices[3]);
}

extern const TVec3f sRotateHammerPressureOffsets[4] = {
    TVec3f(-800.0f, -600.0f, 0.0f),
    TVec3f(605.0f, 800.0f, 0.0f),
    TVec3f(1.0f, 1.0f, 1.0f),
    TVec3f(-600.0f, 450.0f, 0.0f),
};
