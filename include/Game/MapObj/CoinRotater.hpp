#pragma once

#include "Game/NameObj/NameObj.hpp"
#include "JSystem/JGeometry/TMatrix.hpp"

class CoinRotater : public NameObj {

};

namespace MR {
    void createCoinRotater();
    TMtx34f& getCoinInWaterRotateYMatrix();
};

// Merged from Petari (core lane): declarations missing here
#include <JSystem/JGeometry/TMatrix.hpp>

namespace MR {
    TMtx34f& getCoinRotateYMatrix();
    TMtx34f& getCoinHiSpeedRotateYMatrix();
};  // namespace MR
