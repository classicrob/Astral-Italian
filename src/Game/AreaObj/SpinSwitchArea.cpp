#include "Game/AreaObj/SpinSwitchArea.hpp"
#include "Game/Util.hpp"

// PLACEHOLDER(objects): unnamed target symbol fn_8004FEF0, declared extern "C" so the reloc name matches; resolve when fn_8004FEF0 gets its real name in symbols.txt
extern "C" bool fn_8004FEF0();

SpinSwitchArea::SpinSwitchArea(const char* pName) : AreaObj(pName) {
}

void SpinSwitchArea::init(const JMapInfoIter& rIter) {
    MR::createSwingActionWatcher();
    AreaObj::init(rIter);
    MR::connectToSceneAreaObj(this);
}

void SpinSwitchArea::movement() {
    if (!fn_8004FEF0()) {
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
