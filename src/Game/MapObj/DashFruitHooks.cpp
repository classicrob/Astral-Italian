#include "Game/MapObj/DashFruit.hpp"

// The original DashFruit initializer at 0x80293EB0 constructs this exact 0x30-byte
// StemFruitParam block. Its storage remains owned by the original DashFruit TU.
// PLACEHOLDER(m1): original DashFruit parameter storage; resolve when its owner publishes a canonical storage name.
extern StemFruitParam lbl_80729910;

const StemFruitParam* DashFruit::getParam() {
    return &lbl_80729910;
}

void DashFruit::slot0CC() {
}
