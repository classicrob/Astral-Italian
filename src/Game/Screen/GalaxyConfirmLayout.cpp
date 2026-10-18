#include "Game/Screen/GalaxyConfirmLayout.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Screen/BackButton.hpp"
#include "Game/Screen/ButtonPaneController.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"

class GalaxyConfirmBackButton : public BackButton {
public:
    GalaxyConfirmBackButton(const char* name, bool flag) : BackButton(name, flag) { MR::connectToSceneLayoutDecoration(this); }
    virtual ~GalaxyConfirmBackButton();
    virtual void control();
};

namespace NrvGalaxyConfirmLayout {
    NEW_NERVE(GalaxyConfirmLayoutNrvAppear, GalaxyConfirmLayout, Appear);
    NEW_NERVE(GalaxyConfirmLayoutNrvSelecting, GalaxyConfirmLayout, Selecting);
    NEW_NERVE(GalaxyConfirmLayoutNrvSelectedBackButton, GalaxyConfirmLayout, SelectedBackButton);
    NEW_NERVE(GalaxyConfirmLayoutNrvSelected, GalaxyConfirmLayout, Selected);
};  // namespace NrvGalaxyConfirmLayout

GalaxyConfirmLayout::GalaxyConfirmLayout(s32 galaxyType) : LayoutActor("ギャラクシー確認レイアウト", true), mPaneCtrl(nullptr), mBackButton(nullptr), mGalaxyType(galaxyType), mPointingCooldown(0) {
}

void GalaxyConfirmLayout::init(const JMapInfoIter& rIter) {
    initLayoutManager("GalaxySelect", 1);
    MR::connectToSceneLayout(this);

    mPaneCtrl = new ButtonPaneController(this, "StartButton", "BoxStartButton", 0, true);

    if (mGalaxyType == 0) {
        MR::setTextBoxGameMessageRecursive(this, "SButton", "GalaxySelect_StartGalaxy");
    } else if (mGalaxyType == 1) {
        MR::setTextBoxGameMessageRecursive(this, "SButton", "GalaxySelect_StartTico");
    } else if (mGalaxyType == 2) {
        MR::setTextBoxGameMessageRecursive(this, "SButton", "GalaxySelect_StartMarioFace");
    }

    BackButton* pBackButton = new GalaxyConfirmBackButton("戻るボタン", false);
    mBackButton = pBackButton;
    mBackButton->initWithoutIter();

    initNerve(GET_NERVE(GalaxyConfirmLayout, GalaxyConfirmLayoutNrvAppear));
}

void GalaxyConfirmLayout::appear() {
    mPointingCooldown = 0;
    mPaneCtrl->appear();
    mBackButton->appear();
    LayoutActor::appear();
    setNerve(GET_NERVE(GalaxyConfirmLayout, GalaxyConfirmLayoutNrvAppear));
}

bool GalaxyConfirmLayout::isSelected() const {
    return isNerve(GET_NERVE(GalaxyConfirmLayout, GalaxyConfirmLayoutNrvSelected)) ||
           isNerve(GET_NERVE(GalaxyConfirmLayout, GalaxyConfirmLayoutNrvSelectedBackButton));
}

bool GalaxyConfirmLayout::isSelectedYes() const {
    return mPaneCtrl->mIsSelected;
}

void GalaxyConfirmLayout::requestMovementOn() {
    MR::requestMovementOn(this);
    MR::requestMovementOn(mBackButton);
}

void GalaxyConfirmLayout::control() {
    mPaneCtrl->update();
}

void GalaxyConfirmLayout::kill() {
    mBackButton->kill();
    LayoutActor::kill();
}

bool GalaxyConfirmLayout::isSelectionComplete() const {
    if (isNerve(GET_NERVE(GalaxyConfirmLayout, GalaxyConfirmLayoutNrvSelected)) && MR::isDead(this)) {
        return true;
    }
    return false;
}

void GalaxyConfirmLayout::registerMovePositions() {
    if (StarPointerUtil::beginMovePositionRegistration(this, true)) {
        StarPointerUtil::addStarPointerMovePositionFromPane(this, "BoxStartButton", StarPointerUtil::getDefaultButtonOffsetVec2());
        StarPointerUtil::addStarPointerMovePositionFromPane(mBackButton, "BoxButton", StarPointerUtil::getDefaultButtonOffsetVec2());
        StarPointerUtil::setFirstMovePosition("BoxButton");
        StarPointerUtil::setLastMovePosition("BoxStartButton");
        StarPointerUtil::setConnectionMovePositionRight2Way("BoxButton", "BoxStartButton");
        StarPointerUtil::setConnectionMovePositionRight2Way("BoxStartButton", "BoxButton");
    }
    StarPointerUtil::endMovePositionRegistration(this);
}

void GalaxyConfirmLayout::exeAppear() {
    if (!mBackButton->isAppearing() && !mPaneCtrl->isAppearing()) {
        setNerve(GET_NERVE(GalaxyConfirmLayout, GalaxyConfirmLayoutNrvSelecting));
    }
}

void GalaxyConfirmLayout::exeSelecting() {
    registerMovePositions();
    if (mBackButton->isSelected()) {
        setNerve(GET_NERVE(GalaxyConfirmLayout, GalaxyConfirmLayoutNrvSelectedBackButton));
        return;
    }
    if (mPaneCtrl->isPointingTrigger()) {
        MR::startSystemSE("SE_SY_BUTTON_CURSOR_ON");
    }
    if (mPaneCtrl->trySelect()) {
        MR::startSystemSE("SE_SY_GALAXY_DECIDE_OK");
        mBackButton->disappear();
        setNerve(GET_NERVE(GalaxyConfirmLayout, GalaxyConfirmLayoutNrvSelected));
        return;
    }
    if (mBackButton->isPointing()) {
        mPointingCooldown = 30;
        return;
    }
    if (mPointingCooldown > 10) {
        --mPointingCooldown;
    }
}

void GalaxyConfirmLayout::exeSelectedBackButton() {
    if (mBackButton->isDisappearing()) {
        mPaneCtrl->disappear();
        setNerve(GET_NERVE(GalaxyConfirmLayout, GalaxyConfirmLayoutNrvSelected));
    }
}

void GalaxyConfirmLayout::exeSelected() {
    if (mBackButton->isHidden() && mPaneCtrl->isHidden()) {
        kill();
    }
}

void GalaxyConfirmBackButton::control() {
    if (_25 && (MR::testCorePadTriggerB(0) || MR::testCorePadTriggerDown(0)) && !_24 && mPaneCtrl->trySelectImmediately()) {
        MR::startSystemSE("SE_SY_GALAXY_DECIDE_CANCEL");
        _24 = true;
    }
    BackButton::control();
}

GalaxyConfirmBackButton::~GalaxyConfirmBackButton() {}
