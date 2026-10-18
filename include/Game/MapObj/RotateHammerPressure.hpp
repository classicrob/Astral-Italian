#pragma once
#include "Game/MapObj/RotateMoveObj.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>
class RotateHammerPressure : public RotateMoveObj {
public:
    RotateHammerPressure(const char*);
    virtual ~RotateHammerPressure();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    TPos3f mMatrices[4];
    f32 _190;
    f32 _194;
    f32 _198;
    f32 _19C;
    f32 _1A0;
    f32 _1A4;
    bool _1A8;
    bool _1A9;
    s32 _1AC;
};
