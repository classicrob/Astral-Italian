#include "Game/Screen/FileSelectButton.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Screen/ButtonPaneController.hpp"
#include "Game/Screen/GalaxyMapGalaxyPlain.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace NrvFileSelectButton {
    NEW_NERVE(FileSelectButtonNrvSelect, FileSelectButton, Select);
    NEW_NERVE(FileSelectButtonNrvDecided, FileSelectButton, Decided);
    NEW_NERVE(FileSelectButtonNrvWait, FileSelectButton, Wait);
    NEW_NERVE(FileSelectButtonNrvDisappear, FileSelectButton, Disappear);
}
using namespace NrvFileSelectButton;

FileSelectButton::FileSelectButton(const char* pName) : LayoutActor(pName, true), mSelectedButton(0), mHelpText(nullptr) {
    for (int i = 0; i < ARRAY_SIZE(mButtonCtrl); i++) {
        mButtonCtrl[i] = nullptr;
        mCallbackFunctor[i] = nullptr;
        mSelectCallback[i] = nullptr;
    }
}

void FileSelectButton::init(const JMapInfoIter& rIter) {
    initLayoutManager("FileSelect", 1);
    createPaneControl();
    createButtonController();
    createOthers();
    MR::connectToSceneLayout(this);
    initNerve(GET_NERVE_ANON(FileSelectButtonNrvSelect));
}

void FileSelectButton::appear() {
    LayoutActor::appear();

    for (int i = 0; i < ARRAY_SIZE(mButtonCtrl); i++) {
        mButtonCtrl[i]->appear();
    }

    setNerve(GET_NERVE_ANON(FileSelectButtonNrvSelect));
}

void FileSelectButton::kill() {
    LayoutActor::kill();
}

void FileSelectButton::disappear() {
    for (int i = 0; i < ARRAY_SIZE(mButtonCtrl); i++) {
        mButtonCtrl[i]->disappear();
    }

    setNerve(GET_NERVE_ANON(FileSelectButtonNrvDisappear));
}

void FileSelectButton::setCallbackFunctor(const MR::FunctorBase& start, const MR::FunctorBase& mii,
                                          const MR::FunctorBase& remove, const MR::FunctorBase& manual) {
    mCallbackFunctor[0] = start.clone(nullptr);
    mCallbackFunctor[1] = mii.clone(nullptr);
    mCallbackFunctor[2] = remove.clone(nullptr);
    mCallbackFunctor[3] = manual.clone(nullptr);
}

void FileSelectButton::shiftSelect() {
    setNerve(GET_NERVE_ANON(FileSelectButtonNrvSelect));
}

void FileSelectButton::exeSelect() {
    for (int i = 0; i < ARRAY_SIZE(mButtonCtrl); i++) {
        if (mButtonCtrl[i]->isPointingTrigger()) {
            MR::startSystemSE("SE_SY_BUTTON_CURSOR_ON");
        }
        mButtonCtrl[i]->trySelect();
        if (mSelectCallback[i] && mSelectCallback[i]()) {
            mButtonCtrl[i]->trySelectImmediately();
        }
        if (mButtonCtrl[i]->mIsSelected) {
            mSelectedButton = i;
            switch (i) {
            case 0:
                MR::startSystemSE("SE_SY_GALAXY_SELECTED");
                MR::startSubBGM("BGM_SMG2_JG_DECIDE", false);
                break;
            case 1:
                MR::startSystemSE("SE_SY_FILE_SEL_UPPER_DECIDE");
                break;
            case 2:
                MR::startSystemSE("SE_SY_FILE_SEL_UPPER_DECIDE");
                break;
            case 3:
                MR::startSystemSE("SE_SY_FILE_SEL_TIPS_OPEN");
                break;
            }
            setNerve(GET_NERVE_ANON(FileSelectButtonNrvDecided));
            break;
        }
    }
}

void FileSelectButton::exeDecided() {
    if (mButtonCtrl[mSelectedButton]->isDecidedWait()) {
        if (mCallbackFunctor[mSelectedButton]) {
            (*mCallbackFunctor[mSelectedButton])();
        }
        setNerve(GET_NERVE_ANON(FileSelectButtonNrvWait));
    }
}

void FileSelectButton::exeWait() {
}

void FileSelectButton::exeDisappear() {
    for (int i = 0; i < ARRAY_SIZE(mButtonCtrl); i++) {
        if (!mButtonCtrl[i]->isHidden()) {
            return;
        }
    }

    kill();
}

void FileSelectButton::control() {
    for (int i = 0; i < ARRAY_SIZE(mButtonCtrl); i++) {
        mButtonCtrl[i]->update();
    }

    if (mButtonCtrl[3]->isPointing()) {
        mHelpText->show(MR::getGameMessageDirect("2PGuidanceIcon"), "P2Button");
    }
}

void FileSelectButton::createPaneControl() {
    MR::createAndAddPaneCtrl(this, "MiiButton", 1);
    MR::createAndAddPaneCtrl(this, "DeleteButton", 1);
    MR::createAndAddPaneCtrl(this, "StartButton", 1);
    MR::createAndAddPaneCtrl(this, "P2ManualButton", 1);
}

void FileSelectButton::createButtonController() {
    mButtonCtrl[1] = new ButtonPaneController(this, "MiiButton", "BoxMii", 0, true);
    mButtonCtrl[2] = new ButtonPaneController(this, "DeleteButton", "BoxDelete", 0, true);
    mButtonCtrl[0] = new ButtonPaneController(this, "StartButton", "BoxStartButton", 0, true);
    mButtonCtrl[3] = new ButtonPaneController(this, "P2ManualButton", "BoxP2", 0, true);

    for (int i = 0; i < ARRAY_SIZE(mButtonCtrl); i++) {
        mButtonCtrl[i]->_22 = false;
    }
}

void FileSelectButton::createOthers() {
    mHelpText = new GalaxyMapGalaxyPlain(this);
    mHelpText->initWithoutIter();

    MR::connectToSceneLayoutDecoration(mHelpText);
}
