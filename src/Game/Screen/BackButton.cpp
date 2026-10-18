#include "Game/Screen/BackButton.hpp"
#include "Game/Util.hpp"
#include "Game/Screen/ButtonPaneController.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

BackButton::BackButton(const char* pName, bool param2) : LayoutActor(pName, true), mPaneCtrl(), _24(), _25(true), mWaitForSelection(false) {
    if (param2) {
        MR::connectToSceneLayoutFileSelect(this);
    }
}

void BackButton::appear() {
    LayoutActor::appear();
    mPaneCtrl->appear();

    _24 = false;
}

void BackButton::disappear() {
    mPaneCtrl->disappear();
}

bool BackButton::isHidden() const {
    return mPaneCtrl->isHidden();
}

bool BackButton::isAppearing() const {
    return mPaneCtrl->isAppearing();
}

bool BackButton::isDisappearing() const {
    return mPaneCtrl->isDisappearing();
}

bool BackButton::isPointing() const {
    return mPaneCtrl->isPointing();
}

BackButton::~BackButton() {
}

void BackButton::init(const JMapInfoIter& rIter) {
    initLayoutManager("BackButton", 1);
    MR::createAndAddPaneCtrl(this, "Back", 1);

    mPaneCtrl = new ButtonPaneController(this, "Back", "BoxButton", 0, true);
}

void BackButton::control() {
    if (mPaneCtrl->isPointingTrigger()) {
        MR::startSystemSE("SE_SY_BUTTON_CURSOR_ON");
    }

    if (_25 && mPaneCtrl->trySelect()) {
        if (!_24) {
            MR::startSystemSE("SE_SY_GALAXY_DECIDE_CANCEL");
        }

        _24 = true;
    }

    mPaneCtrl->update();
}

bool BackButton::isSelected() const {
    if (mWaitForSelection) {
        bool selected = false;
        if (_24 && mPaneCtrl->isDecidedWait()) {
            selected = true;
        }
        return selected;
    }
    return _24;
}

void BackButton::waitForSelection() {
    mPaneCtrl->_22 = false;
    mWaitForSelection = true;
}
