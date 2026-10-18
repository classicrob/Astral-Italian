#include "Game/AreaObj/YoshiEatGuidanceArea.hpp"
#include "Game/Util.hpp"

// PLACEHOLDER(objects): unnamed target symbol fn_80422710, declared extern "C" so the reloc name matches; resolve when fn_80422710 gets its real name in symbols.txt
extern "C" bool fn_80422710();

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

    if (fn_80422710()) {
        onSwitchA();
    } else {
        offSwitchA();
    }
}
