#include "Game/AreaObj/CumulativeTimerSwitchArea.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util.hpp"

CumulativeTimerSwitchArea::CumulativeTimerSwitchArea(const char* pName) : AreaObj(pName) {
}

void CumulativeTimerSwitchArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
    mTimer = mObjArg0;

    if (GameSequenceFunction::isAllowDreamerInStage() && !GameSequenceFunction::isPadMode2()) {
        MR::connectToSceneAreaObj(this);
    }
}

void CumulativeTimerSwitchArea::movement() {
    if (!isInVolume(*MR::getPlayerPos())) {
        return;
    }

    if (mTimer <= 0) {
        return;
    }

    mTimer--;
    if (mTimer == 0) {
        onSwitchA();
    }
}
