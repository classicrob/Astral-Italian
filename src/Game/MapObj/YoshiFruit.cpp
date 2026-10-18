#include "Game/MapObj/YoshiFruit.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
namespace {
const StemFruitParam sParam("YoshiFruitStem", "YoshiFruit", false, 0, false, true, true, 100, 0.0f, 50.0f, 0.0f, true, "Get");
}
const StemFruitParam* YoshiFruit::getParam() { return &sParam; }
void YoshiFruit::slot0CC() {}
void YoshiFruit::onEat() { MR::incYoshiFruitEaten(); }
void YoshiFruit::onEatEnd() { MR::emitItemYoshiEat(this); }
YoshiFruit::~YoshiFruit() {}
