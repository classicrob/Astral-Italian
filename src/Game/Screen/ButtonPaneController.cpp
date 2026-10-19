#include "Game/Screen/ButtonPaneController.hpp"
#include "JSystem/J3DGraphAnimator/J3DAnimation.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/NativeUiObserve.hpp"

namespace {
    const f32 cDecidedToDisappearAnimRate = 1.5f;
};  // namespace

namespace NrvButtonPaneController {
    NEW_NERVE(ButtonPaneControllerNrvHidden, ButtonPaneController, Hidden);
    NEW_NERVE(ButtonPaneControllerNrvAppear, ButtonPaneController, Appear);
    NEW_NERVE(ButtonPaneControllerNrvWait, ButtonPaneController, Wait);
    NEW_NERVE(ButtonPaneControllerNrvPointing, ButtonPaneController, Pointing);
    NEW_NERVE(ButtonPaneControllerNrvNotPointing, ButtonPaneController, NotPointing);
    NEW_NERVE(ButtonPaneControllerNrvDecided, ButtonPaneController, Decided);
    NEW_NERVE(ButtonPaneControllerNrvDecidedWait, ButtonPaneController, DecidedWait);
    NEW_NERVE(ButtonPaneControllerNrvDecidedToDisappear, ButtonPaneController, DecidedToDisappear);
    NEW_NERVE(ButtonPaneControllerNrvDecidedRepeat, ButtonPaneController, DecidedRepeat);
    NEW_NERVE(ButtonPaneControllerNrvDecidedRepeatWithPad, ButtonPaneController, DecidedRepeatWithPad);
    NEW_NERVE(ButtonPaneControllerNrvDisappear, ButtonPaneController, Disappear);
};  // namespace NrvButtonPaneController

ButtonPaneController::ButtonPaneController(LayoutActor* pHost, const char* pPaneName, const char* pBoundingPaneName, u32 animIndex, bool param5)
    : NerveExecutor(pPaneName), mHost(pHost), mPaneName(pPaneName), mBoundingPaneName(pBoundingPaneName), mAnimIndex(animIndex), mOffAnimIndex(0), mDecidedToDisappear(param5),
      mPointingAnimStartFrame(0.0f), mIsSelected(false), mIsPointing(false), mDisappearOnDecide(true), mIsPointingExternal(false), mIsDisabled(false), mRepeatPadTest(0),
      mAppearAnimName("ButtonAppear"), mWaitAnimName("ButtonWait"), mDecideAnimName("ButtonDecide"), mPointingAnimName("ButtonSelectIn"),
      mNotPointingAnimName("ButtonSelectOut"), mDisappearAnimName("ButtonEnd"), mOffAnimName(nullptr), mFrameCtrl(nullptr) {
    if (!MR::isExistPaneCtrl(pHost, pPaneName)) {
        MR::createAndAddPaneCtrl(pHost, pPaneName, 1);
    }

    MR::hidePane(mHost, mPaneName);
    initNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvHidden));
}

void ButtonPaneController::update() {
    if (mIsActive) {
        updateNerve();

        if (mIsDisabled) {
            setNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvWait));
            return;
        }
    }

    mIsPointing = false;
}

void ButtonPaneController::appear() {
    mPointingAnimStartFrame = 0.0f;
    mIsSelected = false;
    mIsPointing = false;
    mIsActive = true;

    if (mAppearAnimName != nullptr) {
        setNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvAppear));
    } else {
        MR::showPane(mHost, mPaneName);
        setNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvWait));
    }
}

void ButtonPaneController::disappear() {
    mIsActive = true;

    setNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDisappear));
}

bool ButtonPaneController::trySelect() {
    if (!mIsActive) {
        return false;
    }

    mIsPointing = true;

    if (mDecideAnimName != nullptr && isPointing() && MR::testDPDMenuPadDecideTrigger()) {
        mIsSelected = true;

        setNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDecided));

        return true;
    }

    return false;
}

bool ButtonPaneController::trySelectOnPointing() {
    if (!mIsActive) return false;
    mIsPointing = true;
    if (mDecideAnimName != nullptr && isPointing() && MR::testDPDMenuPadDecideTrigger()) {
        mIsSelected = true;
        setNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDecidedRepeat));
        return true;
    }
    return false;
}

bool ButtonPaneController::trySelectWithPad(bool (*testPad)()) {
    if (mIsDisabled) return false;
    mRepeatPadTest = testPad;
    mIsPointing = true;
    mIsSelected = true;
    setNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDecidedRepeatWithPad));
    return true;
}

bool ButtonPaneController::trySelectImmediately() {
    if (!mIsActive) {
        return false;
    }
    mIsPointing = true;
    if (mDecideAnimName != nullptr &&
        (isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvWait)) ||
         isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvPointing)) ||
         isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvNotPointing)))) {
        mIsSelected = true;
        setNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDecided));
        return true;
    }
    return false;
}

#ifdef GALAXY_NATIVE
void ButtonPaneController::publishUiTarget(const char* pId, s32 index) const {
    if (!petari_ui_observing() || isHidden()) {
        return;
    }

    u32 flags = 0;
    if (isPointing()) {
        flags |= PETARI_UI_POINTING;
    }

    // trySelect accepts A only when active, with a decide animation, in the Pointing nerve;
    // Wait and NotPointing move to Pointing when the pointer is over the bounding pane.
    bool isResting = isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvWait)) ||
                     isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvPointing)) ||
                     isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvNotPointing));
    if (mIsActive && mDecideAnimName != nullptr && isResting) {
        flags |= PETARI_UI_SELECTABLE;
    }

    MR::Native::publishUiPaneTarget(mHost, mBoundingPaneName, pId, index, flags);
}
#endif

bool ButtonPaneController::isHidden() const {
    return isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvHidden));
}

bool ButtonPaneController::isPointing() const {
    return isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvPointing));
}

bool ButtonPaneController::isPointingTrigger() const {
    return isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvPointing)) && MR::isFirstStep(this);
}

bool ButtonPaneController::isSelectedRepeat() const {
    return isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDecidedRepeat)) ||
        isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDecidedRepeatWithPad));
}

void ButtonPaneController::setPointingAnimIndex(u32 index) {
    mOffAnimIndex = index;
    mOffAnimName = "ButtonOff";
}

bool ButtonPaneController::isTimingForSelectedSe() const {
    return isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDecided)) && MR::isStep(this, 1);
}

void ButtonPaneController::invalidateDecide() {
    mDecideAnimName = nullptr;
}

void ButtonPaneController::invalidateAppearance() {
    mAppearAnimName = nullptr;
    mDisappearAnimName = nullptr;
    mDisappearOnDecide = false;
}

bool ButtonPaneController::isAppearing() const {
    return isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvAppear));
}

bool ButtonPaneController::isDisappearing() const {
    return isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDisappear));
}

void ButtonPaneController::forceToWait() {
    mPointingAnimStartFrame = 0.0f;
    mIsSelected = false;
    mIsPointing = false;

    setNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvWait));
}

void ButtonPaneController::forceToHide() {
    setNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvHidden));
}

bool ButtonPaneController::isFirstStepWait() const {
    return isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvWait)) && MR::isFirstStep(this);
}

bool ButtonPaneController::isWait() const {
    return isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvWait));
}

bool ButtonPaneController::isDecidedWait() const {
    return isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDecidedWait));
}

bool ButtonPaneController::onPointing() {
    mIsPointing = true;

    return tryPointing(true);
}

void ButtonPaneController::decide() {
    mIsSelected = true;

    setNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDecided));
}

bool ButtonPaneController::isPointingPaneDirect() const {
    return MR::isStarPointerPointingPane(mHost, mBoundingPaneName, 0, true, "弱");
}

bool ButtonPaneController::startAnimAtFirstStep(const char* pAnimName) {
    if (MR::isFirstStep(this)) {
        MR::startPaneAnim(mHost, mPaneName, pAnimName, mAnimIndex);

        return true;
    }

    return false;
}

bool ButtonPaneController::startPointingAnimAtFirstStep(const char* pAnimName) {
    if (startAnimAtFirstStep(pAnimName)) {
        MR::setPaneAnimFrame(mHost, mPaneName, mPointingAnimStartFrame, mAnimIndex);

        return true;
    }

    return false;
}

void ButtonPaneController::setNerveAtAnimStopped(const Nerve* pNerve) {
    if (MR::isPaneAnimStopped(mHost, mPaneName, mAnimIndex)) {
        setNerve(pNerve);
    }
}

bool ButtonPaneController::isPointingPane() const {
    return mIsPointing && MR::isStarPointerPointingPane(mHost, mBoundingPaneName, 0, true, "弱");
}

bool ButtonPaneController::tryPointing(bool param1) {
    if (!param1) {
        return false;
    }

    if (isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvWait)) ||
        isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvNotPointing))) {
        if (isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvNotPointing))) {
            mPointingAnimStartFrame = calcPointingAnimStartFrame();
        } else {
            mPointingAnimStartFrame = 0.0f;
        }

        setNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvPointing));

        return true;
    }

    return false;
}

bool ButtonPaneController::tryNotPointing(bool param1) {
    if (param1) {
        return false;
    }

    if (isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvPointing))) {
        if (isNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvPointing))) {
            mPointingAnimStartFrame = calcPointingAnimStartFrame();
        } else {
            mPointingAnimStartFrame = 0.0f;
        }

        setNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvNotPointing));

        return true;
    }

    return false;
}

f32 ButtonPaneController::calcPointingAnimStartFrame() const {
    if (MR::isPaneAnimStopped(mHost, mPaneName, mAnimIndex)) {
        return 0.0f;
    }

    f32 animFrame = MR::getPaneAnimFrame(mHost, mPaneName, mAnimIndex);
    f32 animFrameMax = MR::getAnimFrameMax(mHost, mPointingAnimName);

    animFrame = animFrameMax - animFrame;

    if (animFrame < 0.0f) {
        return 0.0f;
    }

    if (animFrame > animFrameMax) {
        return animFrameMax;
    }

    return animFrame;
}

void ButtonPaneController::exeHidden() {
    if (MR::isFirstStep(this)) {
        MR::hidePane(mHost, mPaneName);
        mIsActive = false;
    }
}

void ButtonPaneController::exeAppear() {
    if (startAnimAtFirstStep(mAppearAnimName)) {
        MR::showPane(mHost, mPaneName);
    }

    setNerveAtAnimStopped(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvWait));
}

// NONMATCHING: host/pane loads are scheduled before the off-animation frame selection.
void ButtonPaneController::exeWait() {
    startAnimAtFirstStep(mWaitAnimName);
    if (mFrameCtrl != nullptr) {
        f32 ratio = mFrameCtrl->getFrame() / mFrameCtrl->getEnd();
        s16 maxFrame = MR::getPaneAnimFrameMax(mHost, mPaneName, mAnimIndex);
        MR::setPaneAnimFrame(mHost, mPaneName, ratio * maxFrame, mAnimIndex);
        MR::setPaneAnimRate(mHost, mPaneName, 0.0f, mAnimIndex);
    }
    if (mOffAnimName != nullptr) {
        f32 frame;
        if (mIsDisabled) frame = 1.0f;
        else frame = 0.0f;
        MR::startPaneAnimAndSetFrameAndStop(mHost, mPaneName, mOffAnimName, frame, mOffAnimIndex);
        if (mIsDisabled) return;
    }
    if (!mIsPointingExternal) {
        if (tryPointing(isPointingPane())) return;
    }
}

void ButtonPaneController::exePointing() {
    startPointingAnimAtFirstStep(mPointingAnimName);

    if (!mIsPointingExternal) {
        if (tryNotPointing(isPointingPane())) {
            return;
        }
    } else if (tryNotPointing(mIsPointing)) {
        return;
    }
}

void ButtonPaneController::exeNotPointing() {
    startPointingAnimAtFirstStep(mNotPointingAnimName);

    if (!mIsPointingExternal) {
        if (tryPointing(isPointingPane())) {
            return;
        }
    }

    setNerveAtAnimStopped(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvWait));
}

void ButtonPaneController::exeDecided() {
    startAnimAtFirstStep(mDecideAnimName);

    if (mDecidedToDisappear) {
        setNerveAtAnimStopped(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDecidedToDisappear));
    } else if (mDisappearOnDecide) {
        setNerveAtAnimStopped(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDisappear));
    } else {
        setNerveAtAnimStopped(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDecidedWait));
    }
}

void ButtonPaneController::exeDecidedWait() {
}

void ButtonPaneController::exeDecidedToDisappear() {
    if (startAnimAtFirstStep(mNotPointingAnimName)) {
        MR::setPaneAnimRate(mHost, mPaneName, ::cDecidedToDisappearAnimRate, mAnimIndex);
    }

    if (mDisappearOnDecide) {
        setNerveAtAnimStopped(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDisappear));
    } else {
        setNerveAtAnimStopped(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvDecidedWait));
    }
}

void ButtonPaneController::exeDisappear() {
    startAnimAtFirstStep(mDisappearAnimName);
    setNerveAtAnimStopped(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvHidden));
}

void ButtonPaneController::exeDecidedRepeat() {
    startAnimAtFirstStep(mDecideAnimName);
    if (!MR::testPlayer1TriggerA()) {
        mPointingAnimStartFrame = MR::getAnimFrameMax(mHost, mPointingAnimName);
        setNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvPointing));
    }
}

void ButtonPaneController::exeDecidedRepeatWithPad() {
    startAnimAtFirstStep(mDecideAnimName);
    if (!mRepeatPadTest()) {
        mRepeatPadTest = nullptr;
        mPointingAnimStartFrame = MR::getAnimFrameMax(mHost, mPointingAnimName);
        setNerve(GET_NERVE(ButtonPaneController, ButtonPaneControllerNrvPointing));
    }
}
