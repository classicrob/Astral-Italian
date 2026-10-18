#include "Game/Screen/InformationMessage.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/IconAButton.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

namespace NrvInformationMessage {
    NEW_NERVE(InformationMessageNrvAppear, InformationMessage, Appear);
    NEW_NERVE(InformationMessageNrvWait, InformationMessage, Wait);
    NEW_NERVE(InformationMessageNrvDisappear, InformationMessage, Disappear);
};  // namespace NrvInformationMessage

InformationMessage::InformationMessage(bool isCenter)
    : LayoutActor("インフォメーションメッセージ", true), mAButtonIcon(nullptr), mWindowType(0), mIsCenter(isCenter), _35(false) {
}

void InformationMessage::init(const JMapInfoIter& rIter) {
    initLayoutManager("InformationWindow", 2);
    MR::createAndAddPaneCtrl(this, "AButtonPosU", 1);
    MR::createAndAddPaneCtrl(this, "AButtonPosC", 1);
    MR::createAndAddPaneCtrl(this, "AButtonPosD", 1);

    if (mIsCenter) {
        MR::connectToSceneLayoutDecoration(this);
    }

    mAButtonIcon = new IconAButton(mIsCenter, false);
    mAButtonIcon->initWithoutIter();

    if (MR::isExistSceneObj(SceneObj_DemoDirector)) {
        MR::registerDemoSimpleCastAll(this);
        MR::registerDemoSimpleCastAll(mAButtonIcon);
    }

    initNerve(GET_NERVE(InformationMessage, InformationMessageNrvAppear));
}

const char* InformationMessage::getInfConfirmName() const {
    if (mWindowType == 0) return "InfConfirmU";
    if (mWindowType == 1) return "InfConfirmC";
    const char* pane = "InfConfirmC";
    if (mWindowType == 2) pane = "InfConfirmD";
    return pane;
}

void InformationMessage::movement() {
    LayoutActor::movement();
    if (!mIsCenter) mAButtonIcon->movement();
}

void InformationMessage::calcAnim() {
    LayoutActor::calcAnim();
    if (!mIsCenter) mAButtonIcon->calcAnim();
}

void InformationMessage::draw() const {
    LayoutActor::draw();
    if (!mIsCenter) mAButtonIcon->draw();
}

void InformationMessage::kill() {
    mAButtonIcon->kill();
    LayoutActor::kill();
}

void InformationMessage::appear() {
    if (mWindowType == 0) {
        MR::showPaneRecursive(this, "InfoWindowUp");
        MR::hidePaneRecursive(this, "InfoWindowCenter");
        MR::hidePaneRecursive(this, "InfoWindowDown");
    } else if (mWindowType == 1) {
        MR::hidePaneRecursive(this, "InfoWindowUp");
        MR::showPaneRecursive(this, "InfoWindowCenter");
        MR::hidePaneRecursive(this, "InfoWindowDown");
    } else if (mWindowType == 2) {
        MR::hidePaneRecursive(this, "InfoWindowUp");
        MR::hidePaneRecursive(this, "InfoWindowCenter");
        MR::showPaneRecursive(this, "InfoWindowDown");
    }

    MR::startAnim(this, "Appear", 0);
    MR::startAnim(this, "Line", 1);
    MR::setAnimFrameAndStopAdjustTextHeight(this, getInfConfirmName(), 1);
    _35 = false;
    setNerve(GET_NERVE(InformationMessage, InformationMessageNrvAppear));
    LayoutActor::appear();
}

void InformationMessage::setAButtonPos() {
    appear();
    const char* pane;
    if (mWindowType == 0) pane = "AButtonPosU";
    else if (mWindowType == 1) pane = "AButtonPosC";
    else {
        pane = "AButtonPosC";
        if (mWindowType == 2) pane = "AButtonPosD";
    }
    mAButtonIcon->setFollowActorPane(this, pane);
    mAButtonIcon->openWithoutMessage();
    _35 = true;
    MR::requestMovementOn(mAButtonIcon);
}

void InformationMessage::disappear() {
    MR::startSystemSE("SE_SY_TALK_OK", -1, -1);
    MR::startCSSound("CS_CLICK_CLOSE", nullptr, 0, 0, 1.0f);
    setNerve(GET_NERVE(InformationMessage, InformationMessageNrvDisappear));
}

void InformationMessage::setMessage(const char* pMessageId) {
    MR::setTextBoxGameMessageRecursive(this, nullptr, pMessageId);
}

void InformationMessage::setMessage(const wchar_t* pMessage) {
    MR::setTextBoxMessageRecursive(this, nullptr, pMessage);
}

void InformationMessage::setReplaceString(const wchar_t* pMessage, s32 param2) {
    MR::setTextBoxArgStringRecursive(this, nullptr, pMessage, param2);
}

void InformationMessage::exeAppear() {
    MR::setNerveAtAnimStopped(this, GET_NERVE(InformationMessage, InformationMessageNrvWait), 0);
}

void InformationMessage::exeWait() {
    MR::startAnimAtFirstStep(this, "Wait", 0);

    if (!_35) {
        return;
    }

    if (!MR::testCorePadTriggerA(WPAD_CHAN0)) {
        return;
    }

    MR::startSystemSE("SE_SY_TALK_OK", -1, -1);
    MR::startCSSound("CS_CLICK_CLOSE", nullptr, 0, 0, 1.0f);
    setNerve(GET_NERVE(InformationMessage, InformationMessageNrvDisappear));
}

void InformationMessage::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "End", 0);

        if (!MR::isDead(mAButtonIcon)) {
            mAButtonIcon->term();
        }
    }

    if (MR::isAnimStopped(this, 0) && MR::isDead(mAButtonIcon)) {
        kill();
    }
}
