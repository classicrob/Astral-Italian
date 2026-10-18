#include "Game/Player/YoshiFruitCounter.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/EventUtil.hpp"
YoshiFruitCounter::YoshiFruitCounter() : NameObj("フルーツカウンタ"),mCount(0),mLifeDelay(0) {
    MR::connectToSceneMapObjMovement(this);
}
YoshiFruitCounter::~YoshiFruitCounter() {}
void YoshiFruitCounter::movement() {
    if(mLifeDelay && --mLifeDelay == 0) MR::addPlayerLeft(1);
}
void YoshiFruitCounter::increment() {
    if(++mCount % 10 == 0) mLifeDelay=30;
}
