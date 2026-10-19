#include "Game/Screen/Manual2P.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Screen/BackButton.hpp"
#include "Game/Screen/ButtonPaneController.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include <cstdio>

namespace NrvManual2P {
    NEW_NERVE(Manual2PNrvAppear, Manual2P, Appear);
    NEW_NERVE(Manual2PNrvWait, Manual2P, Wait);
    NEW_NERVE(Manual2PNrvScrollRight, Manual2P, ScrollRight);
    NEW_NERVE(Manual2PNrvScrollRightAfter, Manual2P, ScrollRightAfter);
    NEW_NERVE(Manual2PNrvPreviousPageOver, Manual2P, PreviousPageOver);
    NEW_NERVE(Manual2PNrvScrollLeft, Manual2P, ScrollLeft);
    NEW_NERVE(Manual2PNrvScrollLeftAfter, Manual2P, ScrollLeftAfter);
    NEW_NERVE(Manual2PNrvNextPageOver, Manual2P, NextPageOver);
    NEW_NERVE(Manual2PNrvDisappear, Manual2P, Disappear);
};  // namespace

#define MANUAL_NERVE(name) (&NrvManual2P::name::sInstance)

namespace {
    s32 countMessage(const char* pName) {
        s32 count = 0;
        char messageID[0x80];

        do {
            snprintf(messageID, sizeof(messageID), "%s%03d", pName, count + 1);
            count++;
        } while (MR::isExistGameMessage(messageID));

        return count;
    }
};  // namespace

Manual2P::Manual2P(const char* pName)
    : LayoutActor(pName, true), mPageIndex(), mPageNum(), mLeftPaneCtrl(), mRightPaneCtrl(), mIsLeftEnabled(true), mIsRightEnabled(true), mBackButton(), mButtonFrameCtrl() {
}

void Manual2P::init(const JMapInfoIter& rIter) {
    initLayoutManager("P2Manual", 2);
    MR::invalidateParentAnim(this);
    MR::createAndAddPaneCtrl(this, "LeftButton", 2);

    mLeftPaneCtrl = new ButtonPaneController(this, "LeftButton", "BoxLButton", 0, true);
    mLeftPaneCtrl->mDisappearOnDecide = 0;

    MR::createAndAddPaneCtrl(this, "RightButton", 2);

    mRightPaneCtrl = new ButtonPaneController(this, "RightButton", "BoxRButton", 0, true);
    mRightPaneCtrl->mDisappearOnDecide = 0;

    mBackButton = new BackButtonCancelB("マニュアルの戻るボタン", true);
    mBackButton->initWithoutIter();
    mBackButton->waitForSelection();

    initNerve(MANUAL_NERVE(Manual2PNrvAppear));
    MR::connectToSceneLayoutTemporarily(this);

    mPageNum = ::countMessage("2PGuidance");
    MR::startPaneAnimAndSetFrameAndStop(this, "RightButton", "ButtonWait", 0.0f, 0);
    mButtonFrameCtrl = new J3DFrameCtrl;
    mButtonFrameCtrl->init(static_cast<s16>(MR::getPaneAnimFrameMax(this, "RightButton", 0)));
    mLeftPaneCtrl->mFrameCtrl = mButtonFrameCtrl;
    mRightPaneCtrl->mFrameCtrl = mButtonFrameCtrl;
}

void Manual2P::appear() {
    LayoutActor::appear();
    setNerve(MANUAL_NERVE(Manual2PNrvAppear));

    mPageIndex = 0;

    MR::startAnim(this, "Picture", 1);

    if (mPageNum > MR::getAnimCtrl(this, 1)->getEnd()) {
        mPageNum = MR::getAnimCtrl(this, 1)->getEnd();
    }
}

bool Manual2P::isClosed() const {
    return MR::isDead(this);
}

void Manual2P::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "Appear", 0);
        mLeftPaneCtrl->appear();
        mRightPaneCtrl->appear();
        mBackButton->appear();
        reflectPageIndex();
    }

    if (MR::isAnimStopped(this, 0) && mLeftPaneCtrl->isWait() && mRightPaneCtrl->isWait()) {
        setNerve(MANUAL_NERVE(Manual2PNrvWait));
    }
}

void Manual2P::exeWait() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "Wait", 0);
    if (mIsLeftEnabled) {
        mLeftPaneCtrl->trySelect();
        if (MR::testSubPadStickTriggerLeft(WPAD_CHAN0) || MR::testCorePadTriggerLeft(WPAD_CHAN0)) mLeftPaneCtrl->trySelectImmediately();
        if (mLeftPaneCtrl->mIsSelected) {
            MR::startSystemSE("SE_SY_FILE_SEL_TIPS_PAGE");
            setNerve(MANUAL_NERVE(Manual2PNrvScrollLeft));
            return;
        }
    } else if (MR::testSubPadStickTriggerLeft(WPAD_CHAN0) || MR::testCorePadTriggerLeft(WPAD_CHAN0)) {
        setNerve(MANUAL_NERVE(Manual2PNrvNextPageOver));
        return;
    }
    if (mIsRightEnabled) {
        mRightPaneCtrl->trySelect();
        if (MR::testSubPadStickTriggerRight(WPAD_CHAN0) || MR::testCorePadTriggerRight(WPAD_CHAN0)) mRightPaneCtrl->trySelectImmediately();
        if (mRightPaneCtrl->mIsSelected) {
            MR::startSystemSE("SE_SY_FILE_SEL_TIPS_PAGE");
            setNerve(MANUAL_NERVE(Manual2PNrvScrollRight));
            return;
        }
    } else if (MR::testSubPadStickTriggerRight(WPAD_CHAN0) || MR::testCorePadTriggerRight(WPAD_CHAN0)) {
        setNerve(MANUAL_NERVE(Manual2PNrvPreviousPageOver));
        return;
    }
    if (mBackButton->isSelected()) setNerve(MANUAL_NERVE(Manual2PNrvDisappear));
}

void Manual2P::exeScrollRight() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "PageOut", 0);
    }

    if (MR::isAnimStopped(this, 0)) {
        setNerve(MANUAL_NERVE(Manual2PNrvScrollRightAfter));
    }
}

void Manual2P::exeScrollRightAfter() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "PageIn", 0);

        mPageIndex++;

        reflectPageIndex();
    }

    if (MR::isAnimStopped(this, 0)) {
        mRightPaneCtrl->mIsActive = true;
        mRightPaneCtrl->forceToWait();
        setNerve(MANUAL_NERVE(Manual2PNrvWait));
    }
}

void Manual2P::exePreviousPageOver() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "PreviousPageOver", 0);
    if (MR::isAnimStopped(this, 0)) setNerve(MANUAL_NERVE(Manual2PNrvWait));
}

void Manual2P::exeScrollLeft() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "PageIn", 0);
        MR::setAnimFrame(this, MR::getAnimCtrl(this, 0)->getEnd() - 1.0f, 0);
        MR::getAnimCtrl(this, 0)->setRate(-1.0f);
    }

    if (MR::getAnimCtrl(this, 0)->getFrame() + MR::getAnimCtrl(this, 0)->getRate() <= 0.0f) {
        setNerve(MANUAL_NERVE(Manual2PNrvScrollLeftAfter));
    }
}

void Manual2P::exeScrollLeftAfter() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "PageOut", 0);
        MR::setAnimFrame(this, MR::getAnimCtrl(this, 0)->getEnd() - 1.0f, 0);
        MR::getAnimCtrl(this, 0)->setRate(-1.0f);

        mPageIndex--;

        reflectPageIndex();
    }

    if (MR::getAnimCtrl(this, 0)->getFrame() - MR::getAnimCtrl(this, 0)->getRate() <= 0.0f) {
        mLeftPaneCtrl->mIsActive = true;
        mLeftPaneCtrl->forceToWait();
        setNerve(MANUAL_NERVE(Manual2PNrvWait));
    }
}

void Manual2P::exeNextPageOver() {
    if (MR::isFirstStep(this)) MR::startAnim(this, "NextPageOver", 0);
    if (MR::isAnimStopped(this, 0)) setNerve(MANUAL_NERVE(Manual2PNrvWait));
}

void Manual2P::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "End", 0);
        reflectPageIndex();
        mBackButton->disappear();
    }

    if (MR::isAnimStopped(this, 0) && mBackButton->isHidden()) {
        kill();
    }
}

void Manual2P::control() {
    mLeftPaneCtrl->update();
    mRightPaneCtrl->update();

    if (mLeftPaneCtrl->isPointingTrigger()) {
        MR::startSystemSE("SE_SY_BUTTON_CURSOR_ON");
    }

    if (mRightPaneCtrl->isPointingTrigger()) {
        MR::startSystemSE("SE_SY_BUTTON_CURSOR_ON");
    }
    if (!isLeftButtonWaiting() && !isRightButtonWaiting()) {
        mButtonFrameCtrl->setFrame(0.0f);
        return;
    }
    mButtonFrameCtrl->update();
}

void Manual2P::reflectPageIndex() {
    MR::startAnim(this, "Picture", 1);

    if (mPageIndex < 0) {
        mPageIndex = 0;
    }

    if (mPageIndex >= mPageNum) {
        mPageIndex = mPageNum - 1;
    }

    MR::setAnimFrameAndStop(this, mPageIndex, 1);

    char messageID[0x80];
    snprintf(messageID, sizeof(messageID), "2PGuidance%03d", mPageIndex + 1);

    if (MR::isExistGameMessage(messageID)) {
        MR::setTextBoxGameMessageRecursive(this, "Text", messageID);
    }

    snprintf(messageID, sizeof(messageID), "2PGuidanceTitle%03d", mPageIndex + 1);

    if (MR::isExistGameMessage(messageID)) {
        MR::setTextBoxGameMessageRecursive(this, "Title", messageID);
    }

    MR::setTextBoxFormatRecursive(this, "PageNumber", L"%d/%d", mPageIndex + 1, mPageNum);

    if (mPageIndex == 0) {
        ButtonPaneController* pPaneCtrl = mLeftPaneCtrl;
        MR::startPaneAnim(this, pPaneCtrl->mPaneName, "ButtonOff", 1);
        MR::setPaneAnimFrameAndStop(this, pPaneCtrl->mPaneName, 1.0f, 1);
        mIsLeftEnabled = false;
    } else {
        ButtonPaneController* pPaneCtrl = mLeftPaneCtrl;
        MR::startPaneAnim(this, pPaneCtrl->mPaneName, "ButtonOff", 1);
        MR::setPaneAnimFrameAndStop(this, pPaneCtrl->mPaneName, 0.0f, 1);
        mIsLeftEnabled = true;
    }

    if (mPageIndex == (mPageNum - 1)) {
        ButtonPaneController* pPaneCtrl = mRightPaneCtrl;
        MR::startPaneAnim(this, pPaneCtrl->mPaneName, "ButtonOff", 1);
        MR::setPaneAnimFrameAndStop(this, pPaneCtrl->mPaneName, 1.0f, 1);
        mIsRightEnabled = false;
    } else {
        ButtonPaneController* pPaneCtrl = mRightPaneCtrl;
        MR::startPaneAnim(this, pPaneCtrl->mPaneName, "ButtonOff", 1);
        MR::setPaneAnimFrameAndStop(this, pPaneCtrl->mPaneName, 0.0f, 1);
        mIsRightEnabled = true;
    }
}



bool Manual2P::isLeftButtonWaiting() const {
    return !mLeftPaneCtrl->isHidden() && mLeftPaneCtrl->isWait() && !MR::isNewNerve(mLeftPaneCtrl);
}
bool Manual2P::isRightButtonWaiting() const {
    return !mRightPaneCtrl->isHidden() && mRightPaneCtrl->isWait() && !MR::isNewNerve(mRightPaneCtrl);
}
