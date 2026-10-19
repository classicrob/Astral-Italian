#include "Game/Screen/BackButton.hpp"
#include "Game/Util.hpp"
#include "Game/Screen/ButtonPaneController.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

BackButton::BackButton(const char* pName, bool param2) : LayoutActor(pName, true), mPaneCtrl(), mIsPressed(), mIsEnabled(true), mWaitForSelection(false) {
    if (param2) {
        MR::connectToSceneLayoutFileSelect(this);
    }
}

void BackButton::appear() {
    LayoutActor::appear();
    mPaneCtrl->appear();

    mIsPressed = false;
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

    if (mIsEnabled && mPaneCtrl->trySelect()) {
        if (!mIsPressed) {
            MR::startSystemSE("SE_SY_GALAXY_DECIDE_CANCEL");
        }

        mIsPressed = true;
    }

    mPaneCtrl->update();
}

bool BackButton::isSelected() const {
    if (mWaitForSelection) {
        bool selected = false;
        if (mIsPressed && mPaneCtrl->isDecidedWait()) {
            selected = true;
        }
        return selected;
    }
    return mIsPressed;
}

void BackButton::waitForSelection() {
    mPaneCtrl->mDisappearOnDecide = false;
    mWaitForSelection = true;
}

BackButtonCancelB::BackButtonCancelB(const char* name, bool connect) : BackButton(name, connect) {}
