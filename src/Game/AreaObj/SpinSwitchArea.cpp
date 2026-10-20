#include "Game/AreaObj/SpinSwitchArea.hpp"
#include "Game/Util.hpp"
#ifdef GALAXY_NATIVE
#include "Game/Player/MarioCappy.hpp"
#include <cstdio>
#include <cstdlib>
#endif


SpinSwitchArea::SpinSwitchArea(const char* pName) : AreaObj(pName) {
}

void SpinSwitchArea::init(const JMapInfoIter& rIter) {
    MR::createSwingActionWatcher();
    AreaObj::init(rIter);
    MR::connectToSceneAreaObj(this);
}

void SpinSwitchArea::movement() {
#ifdef GALAXY_NATIVE
    if (!isValidSwitchA()) {
        return;
    }
#endif
#ifdef GALAXY_NATIVE
    const bool capContact = petariCappyTouchSpinSwitchArea(this);
    const bool playerSpin = MR::isPlayerSwingActionStartNow() && isInVolume(*MR::getPlayerPos());
    if (!capContact && !playerSpin) {
        return;
    }
#else
    if (!MR::isPlayerSwingActionStartNow()) {
        return;
    }

    if (!isValidSwitchA()) {
        return;
    }

    if (!isInVolume(*MR::getPlayerPos())) {
        return;
    }

#endif
#ifdef GALAXY_NATIVE
    if (std::getenv("PETARI_CAPPY_TRACE") != nullptr) {
        std::fprintf(stderr, "[spin-switch] %s source=%s before=%d after=%d\n", mName,
                     capContact ? "cappy" : "spin", isOnSwitchA(), !isOnSwitchA());
    }
#endif
    if (isOnSwitchA()) {
        offSwitchA();
    } else {
        onSwitchA();
    }
}
