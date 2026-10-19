#include "Game/Screen/YesNoController.hpp"
#include "Game/Util.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Screen/ButtonPaneController.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

namespace {
    static const char* sDefaultCursorSE = "SE_SY_TALK_FOCUS_ITEM";
    static const char* sDefaultYesSE = "SE_SY_TALK_SELECT_YES";
    static const char* sDefaultNoSE = "SE_SY_TALK_SELECT_NO";
};  // namespace

namespace NrvYesNoController {
    NEW_NERVE(YesNoControllerNrvSelecting, YesNoController, Selecting);
    NEW_NERVE(YesNoControllerNrvDecided, YesNoController, Decided);
    NEW_NERVE(YesNoControllerNrvDisappear, YesNoController, Disappear);
    NEW_NERVE(YesNoControllerNrvSelected, YesNoController, Selected);
    NEW_NERVE(YesNoControllerNrvNotSelected, YesNoController, NotSelected);
};  // namespace NrvYesNoController

YesNoController::YesNoController(LayoutActor* pHost)
    : NerveExecutor("はい／いいえ選択制御"), mHost(pHost), mIsActive(), mButtonYesPaneCtrl(), mButtonNoPaneCtrl(), mCursorSE(), mYesSE(), mNoSE() {
    mButtonYesPaneCtrl = new ButtonPaneController(mHost, "Right", "BoxRight", 0, true);
    mButtonYesPaneCtrl->mDisappearOnDecide = false;

    mButtonNoPaneCtrl = new ButtonPaneController(mHost, "Left", "BoxLeft", 0, true);
    mButtonNoPaneCtrl->mDisappearOnDecide = false;

    initNerve(GET_NERVE(YesNoController, YesNoControllerNrvSelecting));
}

void YesNoController::appear() {
    mIsActive = true;

    setNerve(GET_NERVE(YesNoController, YesNoControllerNrvSelecting));
}

void YesNoController::kill() {
    mIsActive = false;

    setNerve(GET_NERVE(YesNoController, YesNoControllerNrvNotSelected));
}

void YesNoController::update() {
    if (mIsActive) {
        updateNerve();
        mButtonYesPaneCtrl->update();
        mButtonNoPaneCtrl->update();
    }
}

bool YesNoController::isSelected() const {
    return isNerve(GET_NERVE(YesNoController, YesNoControllerNrvSelected));
}

bool YesNoController::isSelectedYes() const {
    return mButtonYesPaneCtrl->mIsSelected;
}

bool YesNoController::isDisappearStart() const {
    return isNerve(GET_NERVE(YesNoController, YesNoControllerNrvDisappear)) && MR::isFirstStep(this);
}

void YesNoController::setSE(const char* pCursorSE, const char* pYesSE, const char* pNoSE) {
    mCursorSE = pCursorSE;
    mYesSE = pYesSE;
    mNoSE = pNoSE;
}

bool YesNoController::trySelect() {
    if (mButtonYesPaneCtrl->trySelect()) {
        return true;
    }

    return mButtonNoPaneCtrl->trySelect();
}

void YesNoController::emitEffectIfExist(const char* pEffectName) {
    if (MR::isExistEffectKeeper(mHost)) {
        MR::emitEffect(mHost, pEffectName);
    }
}

void YesNoController::deleteEffectIfExist(const char* pEffectName) {
    if (MR::isExistEffectKeeper(mHost)) {
        MR::deleteEffect(mHost, pEffectName);
    }
}

void YesNoController::forceDeleteEffectAllIfExist() {
    if (MR::isExistEffectKeeper(mHost)) {
        MR::forceDeleteEffect(mHost, "LeftText");
        MR::forceDeleteEffect(mHost, "RightText");
    }
}

void YesNoController::exeSelecting() {
    if (MR::isFirstStep(this)) {
        mButtonYesPaneCtrl->appear();
        mButtonNoPaneCtrl->appear();
    }

    registerPointer();
    if (mButtonYesPaneCtrl->isWait()) StarPointerUtil::enableMovePositionInput();

    if (mButtonYesPaneCtrl->isPointingTrigger()) {
        if (mCursorSE != nullptr) {
            MR::startSystemSE(mCursorSE);
        } else {
            MR::startSystemSE(::sDefaultCursorSE);
        }

        forceDeleteEffectAllIfExist();
        emitEffectIfExist("RightText");
    }

    if (mButtonNoPaneCtrl->isPointingTrigger()) {
        if (mCursorSE != nullptr) {
            MR::startSystemSE(mCursorSE);
        } else {
            MR::startSystemSE(::sDefaultCursorSE);
        }

        forceDeleteEffectAllIfExist();
        emitEffectIfExist("LeftText");
    }

    if (!mButtonYesPaneCtrl->isPointing()) {
        deleteEffectIfExist("RightText");
    }

    if (!mButtonNoPaneCtrl->isPointing()) {
        deleteEffectIfExist("LeftText");
    }

    if (trySelect()) {
        setNerve(GET_NERVE(YesNoController, YesNoControllerNrvDecided));
    }
}

void YesNoController::exeDecided() {
    const bool isSelectedYes = mButtonYesPaneCtrl->mIsSelected;

    if (MR::isFirstStep(this)) {
        deleteEffectIfExist("LeftText");
        deleteEffectIfExist("RightText");
        MR::startCSSound("CS_CLICK_CLOSE", nullptr, 0, 0, 1.0f);

        if (isSelectedYes) {
            if (mYesSE != nullptr) {
                MR::startSystemSE(mYesSE);
            } else {
                MR::startSystemSE(::sDefaultYesSE);
            }
        } else {
            if (mNoSE != nullptr) {
                MR::startSystemSE(mNoSE);
            } else {
                MR::startSystemSE(::sDefaultNoSE);
            }
        }
    }

    if ((isSelectedYes && mButtonYesPaneCtrl->isDecidedWait()) || (!isSelectedYes && mButtonNoPaneCtrl->isDecidedWait())) {
        setNerve(GET_NERVE(YesNoController, YesNoControllerNrvDisappear));
    }
}

void YesNoController::exeDisappear() {
    if (MR::isFirstStep(this)) {
        mButtonYesPaneCtrl->disappear();
        mButtonNoPaneCtrl->disappear();
    }

    if (mButtonYesPaneCtrl->isHidden() && mButtonNoPaneCtrl->isHidden()) {
        if (mButtonYesPaneCtrl->mIsSelected || mButtonNoPaneCtrl->mIsSelected) {
            setNerve(GET_NERVE(YesNoController, YesNoControllerNrvSelected));
        } else {
            setNerve(GET_NERVE(YesNoController, YesNoControllerNrvNotSelected));
        }
    }
}

void YesNoController::exeSelected() {
    mIsActive = false;
}

void YesNoController::exeNotSelected() {
    mIsActive = false;
}

bool YesNoController::isYesButtonSelected() const { return mButtonYesPaneCtrl->mIsSelected; }

void YesNoController::registerPointer() {
    if (StarPointerUtil::beginMovePositionRegistration(this, true)) {
        TVec2f left(60.0f, 20.0f);
        StarPointerUtil::addStarPointerMovePositionFromPane(mHost, "BoxLeft", &left);
        TVec2f right(60.0f, 20.0f);
        StarPointerUtil::addStarPointerMovePositionFromPane(mHost, "BoxRight", &right);
        StarPointerUtil::setFirstMovePosition("BoxLeft");
        StarPointerUtil::setLastMovePosition("BoxRight");
        StarPointerUtil::setConnectionMovePositionRight("BoxLeft", "BoxRight");
        StarPointerUtil::disableMovePositionInput();
    }
    StarPointerUtil::endMovePositionRegistration(mHost);
    StarPointerUtil::refreshMovePositionsFromLayout(mHost);
}
