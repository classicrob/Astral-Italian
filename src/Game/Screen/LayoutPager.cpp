#include "Game/Screen/LayoutPager.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Screen/BackButton.hpp"
#include "Game/Screen/ButtonPaneController.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"

namespace {
    ButtonPaneController* createPageButton(LayoutActor* host, const char* pane, const char* box) NO_INLINE {
        ButtonPaneController* button = new ButtonPaneController(host, pane, box, 0, true);
        button->_22 = false;
        button->forceToHide();
        return button;
    }
}

namespace NrvLayoutPager {
    class Idle : public Nerve { public: virtual void execute(Spine*) const {} static Idle sInstance; };
    Idle Idle::sInstance;
    NEW_NERVE(Previous, LayoutPager, Previous);
    NEW_NERVE(PreviousInvalid, LayoutPager, PreviousInvalid);
    NEW_NERVE(Next, LayoutPager, Next);
    NEW_NERVE(NextInvalid, LayoutPager, NextInvalid);
    NEW_NERVE(Wait, LayoutPager, Wait);
    NEW_NERVE(Appear, LayoutPager, Wait);
    NEW_NERVE(Decide, LayoutPager, Decide);
}

LayoutPager::LayoutPager(LayoutActor* host, const char* pane, bool twoPrevious, bool previous, bool next, bool twoNext)
    : ActorStateBase<LayoutActor>("ページャー", host), mPaneName(pane), mNextButton(nullptr), mPreviousButton(nullptr), mBackButton(nullptr),
      mCanShowTwoPrevious(twoPrevious), mCanShowPrevious(previous), mCanShowNext(next), mCanShowTwoNext(twoNext) {
    initNerve(&NrvLayoutPager::Idle::sInstance);
    mNextButton = createPageButton(mHost, "Right", "BoxArrowR");
    mPreviousButton = createPageButton(mHost, "Left", "BoxArrowL");
    BackButton* back = new BackButtonCancelB("戻るボタン", false);
    back->initWithoutIter();
    mBackButton = back;
    refreshButtons();
    MR::connectToScene(mBackButton, MR::MovementType_Layout, MR::CalcAnimType_Layout, MR::DrawBufferType_None, MR::DrawType_LayoutOnPause);
}

#pragma dont_inline on
LayoutPager::~LayoutPager() {}
#pragma dont_inline reset

void LayoutPager::appear() {
    mIsDead = false;
    MR::startStarPointerModeChooseYesNo(this);
    MR::requestMovementOn(mBackButton);
    mBackButton->appear();
    mNextButton->forceToHide();
    mPreviousButton->forceToHide();
    refreshButtons();
    setNerve(&NrvLayoutPager::Appear::sInstance);
}

void LayoutPager::kill() {
    mNextButton->forceToHide();
    mPreviousButton->forceToHide();
    MR::endStarPointerMode(this);
    mIsDead = true;
}

void LayoutPager::updateButtons() {
    mNextButton->update();
    mPreviousButton->update();
    if (mNextButton->isWait() && mNextButton->getNerveStep() == 1 && mPreviousButton->isWait())
        MR::setPaneAnimFrame(mHost, "Right", MR::getPaneAnimFrame(mHost, "Left", 0), 0);
    if (mPreviousButton->isWait() && mPreviousButton->getNerveStep() == 1 && mNextButton->isWait())
        MR::setPaneAnimFrame(mHost, "Left", MR::getPaneAnimFrame(mHost, "Right", 0), 0);
}

bool LayoutPager::isPreviousSelected() const {
    bool selected = false;
    if (isNerve(&NrvLayoutPager::Previous::sInstance) && MR::isStep(this, 10)) selected = true;
    return selected;
}

bool LayoutPager::isNextSelected() const {
    bool selected = false;
    if (isNerve(&NrvLayoutPager::Next::sInstance) && MR::isStep(this, 10)) selected = true;
    return selected;
}

void LayoutPager::refreshButtons() {
    if (mCanShowNext) {
        if (!mNextButton->_24 || mNextButton->isHidden()) mNextButton->appear();
        if (mCanShowTwoNext) mNextButton->_22 = false;
        else mNextButton->_22 = true;
    } else mNextButton->forceToHide();
    if (mCanShowPrevious) {
        if (!mPreviousButton->_24 || mPreviousButton->isHidden()) mPreviousButton->appear();
        else mPreviousButton->forceToWait();
        if (mCanShowTwoPrevious) mPreviousButton->_22 = false;
        else mPreviousButton->_22 = true;
    } else mPreviousButton->forceToHide();
}

bool LayoutPager::isBackSelected() const {
    if (mBackButton->isAppearing()) return false;
    if (mBackButton->isHidden()) return false;
    return mBackButton->isSelected();
}

bool LayoutPager::trySelectPrevious() {
    if (!mPreviousButton->isHidden() && mPreviousButton->trySelect()) return true;
    if (mPreviousButton->mIsSelected) return true;
    return false;
}

bool LayoutPager::trySelectNext() {
    if (!mNextButton->isHidden() && mNextButton->trySelect()) return true;
    if (mNextButton->mIsSelected) return true;
    return false;
}

void LayoutPager::forceButtonsToWait() {
    if (!mNextButton->isWait()) mNextButton->forceToWait();
    if (!mPreviousButton->isWait()) mPreviousButton->forceToWait();
}

void LayoutPager::setPageAvailability(bool twoPrevious, bool previous, bool next, bool twoNext) {
    mCanShowTwoPrevious = twoPrevious;
    mCanShowPrevious = previous;
    mCanShowNext = next;
    mCanShowTwoNext = twoNext;
}

void LayoutPager::exePrevious() {
    if (MR::isFirstStep(this)) MR::startPaneAnim(mHost, mPaneName, "PreviousPage", 0);
    if (MR::isStep(this, 11)) refreshButtons();
    if (MR::isPaneAnimStopped(mHost, mPaneName, 0)) {
        forceButtonsToWait();
        setNerve(&NrvLayoutPager::Wait::sInstance);
    }
}

void LayoutPager::exePreviousInvalid() {
    if (MR::isFirstStep(this)) {
        MR::startPaneAnim(mHost, mPaneName, "PreviousPageOver", 0);
        MR::startActionSound(mHost, "PageChangeInvalid", -1, -1);
    }
    if (MR::isPaneAnimStopped(mHost, mPaneName, 0)) {
        forceButtonsToWait();
        setNerve(&NrvLayoutPager::Wait::sInstance);
    }
}

void LayoutPager::exeNext() {
    if (MR::isFirstStep(this)) MR::startPaneAnim(mHost, mPaneName, "NextPage", 0);
    if (MR::isStep(this, 11)) refreshButtons();
    if (MR::isPaneAnimStopped(mHost, mPaneName, 0)) {
        forceButtonsToWait();
        setNerve(&NrvLayoutPager::Wait::sInstance);
    }
}

void LayoutPager::exeNextInvalid() {
    if (MR::isFirstStep(this)) {
        MR::startPaneAnim(mHost, mPaneName, "NextPageOver", 0);
        MR::startActionSound(mHost, "PageChangeInvalid", -1, -1);
    }
    if (MR::isPaneAnimStopped(mHost, mPaneName, 0)) {
        forceButtonsToWait();
        setNerve(&NrvLayoutPager::Wait::sInstance);
    }
}

void LayoutPager::exeWait() {
    if (MR::isFirstStep(this)) MR::startAnim(mHost, "Wait", 0);
    if (isNerve(&NrvLayoutPager::Appear::sInstance) && MR::isStep(this, 11)) forceButtonsToWait();
    bool pointing = false;
    if (mNextButton->isPointingTrigger() || mPreviousButton->isPointingTrigger()) pointing = true;
    if (pointing) MR::startSystemSE("SE_SY_SELECT_PAUSE_ITEM", -1, -1);
    if (isBackSelected()) {
        setNerve(&NrvLayoutPager::Decide::sInstance);
        return;
    }
    if (MR::testSystemTriggerLeft()) {
        if (mPreviousButton->_24) mPreviousButton->trySelectImmediately();
        else setNerve(&NrvLayoutPager::PreviousInvalid::sInstance);
    }
    if (MR::testSystemTriggerRight()) {
        if (mNextButton->_24) mNextButton->trySelectImmediately();
        else setNerve(&NrvLayoutPager::NextInvalid::sInstance);
    }
    bool selected = false;
    if (trySelectPrevious() || trySelectNext()) selected = true;
    if (selected) setNerve(&NrvLayoutPager::Decide::sInstance);
}

void LayoutPager::exeDecide() {
    if (mNextButton->mIsSelected) {
        if (mCanShowNext) setNerve(&NrvLayoutPager::Next::sInstance);
        else setNerve(&NrvLayoutPager::NextInvalid::sInstance);
        return;
    }
    if (mPreviousButton->mIsSelected) {
        if (mCanShowPrevious) setNerve(&NrvLayoutPager::Previous::sInstance);
        else setNerve(&NrvLayoutPager::PreviousInvalid::sInstance);
        return;
    }
    if (mBackButton->isSelected() && mBackButton->isHidden()) kill();
}
