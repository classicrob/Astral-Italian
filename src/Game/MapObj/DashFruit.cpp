#include "Game/MapObj/DashFruit.hpp"
#include "Game/Util/YoshiUtil.hpp"

namespace {
    StemFruitParam sDashFruitParam("DashFruitStem", "DashFruit", true, 240, false, true, true,
                                  100, 15.0f, 50.0f, 1.0f, true, "Get");
}

const StemFruitParam* DashFruit::getParam() {
    return &sDashFruitParam;
}

void DashFruit::slot0CC() {
}

void DashFruit::onEat() {
    MR::startDashYoshiMode();
}

inline void StemFruit::onEatEnd() {
}

DashFruit::~DashFruit() {
}

inline StemFruit::~StemFruit() {
}

