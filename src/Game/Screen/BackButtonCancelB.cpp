#include "Game/Screen/BackButton.hpp"
#include "Game/Screen/ButtonPaneController.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

BackButtonCancelB::~BackButtonCancelB() {}

void BackButtonCancelB::control() {
    if (_25 && MR::testCorePadTriggerB(0) && !_24 && mPaneCtrl->trySelectImmediately()) {
        MR::startSystemSE("SE_SY_GALAXY_DECIDE_CANCEL", -1, -1);
        _24 = true;
    }
    BackButton::control();
}
