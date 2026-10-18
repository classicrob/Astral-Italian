#include "Game/MapObj/YoshiFruit.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
namespace {
const StemFruitParam sParam("YoshiFruitStemBig", "YoshiFruitBig", false, 0, true, false, false, 120, 0.0f, 100.0f, 0.0f, true, "Get");
}
const StemFruitParam* YoshiFruitBig::getParam() { return &sParam; }
void YoshiFruitBig::slot0CC() {}
void YoshiFruitBig::onEat() {}
void YoshiFruitBig::onEatEnd() { MR::emitItemYoshiEat(this); }
YoshiFruitBig::~YoshiFruitBig() {}
