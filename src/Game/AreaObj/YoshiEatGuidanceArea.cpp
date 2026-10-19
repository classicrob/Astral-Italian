#include "Game/AreaObj/YoshiEatGuidanceArea.hpp"
#include "Game/Util/YoshiUtil.hpp"
#include "Game/Util.hpp"


YoshiEatGuidanceArea::YoshiEatGuidanceArea(const char* pName) : AreaObj(pName) {
}

// NONMATCHING (symbol): target name is init__20YoshiEatGuidanceAreaFFRC12JMapInfoIter.
void YoshiEatGuidanceArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    MR::connectToSceneAreaObj(this);
}

void YoshiEatGuidanceArea::movement() {
    if (!isInVolume(*MR::getPlayerPos())) {
        return;
    }

    MR::requestYoshiEatGuidance(mObjArg0 == 0);

    if (!isValidSwitchA()) {
        return;
    }

    if (MR::isYoshiTongueReadyToEat()) {
        onSwitchA();
    } else {
        offSwitchA();
    }
}
