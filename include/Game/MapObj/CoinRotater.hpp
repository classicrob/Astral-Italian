#pragma once

#include "Game/NameObj/NameObj.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class CoinRotater : public NameObj {
public:
    CoinRotater(const char*);

    virtual void movement();

    /* 0x14 */ f32 _C;
    /* 0x18 */ f32 _10;
    /* 0x1C */ f32 _14;
    /* 0x20 */ TMtx34f mRotateYMtx;
    /* 0x50 */ TMtx34f mHiSpeedRotateYMtx;
    /* 0x80 */ TMtx34f mWaterRotateMtx;
};

namespace MR {
    void createCoinRotater();
    TMtx34f& getCoinRotateYMatrix();
    TMtx34f& getCoinHiSpeedRotateYMatrix();
    TMtx34f& getCoinInWaterRotateYMatrix();
};  // namespace MR
