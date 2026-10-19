#include "Game/AreaObj/SpinSwitchArea.hpp"
#include "Game/Util.hpp"


SpinSwitchArea::SpinSwitchArea(const char* pName) : AreaObj(pName) {
}

void SpinSwitchArea::init(const JMapInfoIter& rIter) {
    MR::createSwingActionWatcher();
    AreaObj::init(rIter);
    MR::connectToSceneAreaObj(this);
}

void SpinSwitchArea::movement() {
    if (!MR::isPlayerSwingActionStartNow()) {
        return;
    }

    if (!isValidSwitchA()) {
        return;
    }

    if (!isInVolume(*MR::getPlayerPos())) {
        return;
    }

    if (isOnSwitchA()) {
        offSwitchA();
    } else {
        onSwitchA();
    }
}
