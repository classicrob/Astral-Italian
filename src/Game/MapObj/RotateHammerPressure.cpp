#include "Game/MapObj/RotateHammerPressure.hpp"
RotateHammerPressure::RotateHammerPressure(const char* name)
    : RotateMoveObj(name), _190(0.0f), _194(0.0f), _198(0.0f), _19C(0.0f), _1A0(0.0f), _1A4(0.0f), _1A8(false), _1A9(false), _1AC(0) {
    PSMTXIdentity(mMatrices[0]);
    PSMTXIdentity(mMatrices[1]);
    PSMTXIdentity(mMatrices[2]);
    PSMTXIdentity(mMatrices[3]);
}
