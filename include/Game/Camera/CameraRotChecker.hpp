#pragma once

#include <JSystem/JGeometry/TMatrix.hpp>

class CameraRotChecker {
public:
    CameraRotChecker();

    void update();

    /* 0x00 */ f32 mThreshold;
    /* 0x04 */ TPos3f mMtx;
    /* 0x34 */ bool mIsRotatingHard;
};
