#include "Game/Screen/StarPieceBank.hpp"
#include "Game/Screen/ButtonPaneController.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Screen/SimpleLayout.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Util/ActionSoundUtil.hpp"

namespace {
    bool isPadButtonUpOnly() {
        bool result = false;
        if (MR::testCorePadButtonUp(0) && !MR::testCorePadButtonLeft(0) && !MR::testCorePadButtonRight(0)) result = true;
        return result;
    }
    bool isPadButtonDownOnly() {
        bool result = false;
        if (MR::testCorePadButtonDown(0) && !MR::testCorePadButtonLeft(0) && !MR::testCorePadButtonRight(0)) result = true;
        return result;
    }
}

namespace NrvStarPieceBank {
    class StarPieceBankNrvWait : public Nerve {
    public:
        static StarPieceBankNrvWait sInstance;
        virtual void execute(Spine*) const;
    };
    StarPieceBankNrvWait StarPieceBankNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(StarPieceBankNrvAppear, StarPieceBank, Appear);
    NEW_NERVE(StarPieceBankNrvSelect, StarPieceBank, Select);
    NEW_NERVE(StarPieceBankNrvTradeStart, StarPieceBank, Trade);
    NEW_NERVE(StarPieceBankNrvTrade, StarPieceBank, Trade);
    NEW_NERVE(StarPieceBankNrvConfirm, StarPieceBank, Decided);
    NEW_NERVE(StarPieceBankNrvCancel, StarPieceBank, Decided);
    NEW_NERVE(StarPieceBankNrvDisappear, StarPieceBank, Disappear);
}

StarPieceBank::StarPieceBank(const char* name)
    : LayoutActor(name, true), mExplanation(0), mWithdrawButton(0), mDepositButton(0), mBankedStarPieces(0), mStockedStarPieces(0) {
    initLayoutManager("StarPieceBank", 1);
    MR::createAndAddPaneCtrl(this, "StarPieceCounter", 1);
    MR::createAndAddPaneCtrl(this, "TradeUpButton", 2);
    MR::createAndAddPaneCtrl(this, "TradeDownButton", 2);
    mWithdrawButton = new ButtonPaneController(this, "TradeUpButton", "BoxUButton", 0, true);
    mWithdrawButton->mDisappearOnDecide = false;
    mWithdrawButton->setPointingAnimIndex(1);
    mDepositButton = new ButtonPaneController(this, "TradeDownButton", "BoxDButton", 0, true);
    mDepositButton->mDisappearOnDecide = false;
    mDepositButton->setPointingAnimIndex(1);
    mConfirmButton = new ButtonPaneController(this, "RightButton", "BoxRight", 0, true);
    mConfirmButton->mDisappearOnDecide = false;
    mCancelButton = new ButtonPaneController(this, "LeftButton", "BoxLeft", 0, true);
    mCancelButton->mDisappearOnDecide = false;
    MR::connectToSceneLayoutTemporarily(this);
    mExplanation = new SimpleTalkBalloon("銀行説明バルーン", "TalkBalloonEvent", 1);
    mExplanation->kill();
    initActionSound(3, "StarPieceBank", 0);
    initNerve(&NrvStarPieceBank::StarPieceBankNrvSelect::sInstance);
    kill();
}

void StarPieceBank::appear() {
    mBankedStarPieces = GameDataFunction::getBankedStarPieceNum();
    mStockedStarPieces = GameDataFunction::getStockedStarPieceNum();
    updateCounters();
    if (MR::isPlayerLuigi()) MR::hidePaneRecursive(this, "ShaMario");
    else MR::hidePaneRecursive(this, "ShaLuigi");
    MR::tryFrameToScreenCinemaFrame();
    MR::requestMovementOn(this);
    MR::requestMovementOn(mExplanation);
    MR::requestPauseBlurMovementOn();
    MR::startAnimAndSetFrameAndStop(this, "Appear", 0.0f, 0);
    MR::hideLayout(this);
    setNerve(&NrvStarPieceBank::StarPieceBankNrvWait::sInstance);
    LayoutActor::appear();
}

void StarPieceBank::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::incrementPauseBlurDelay();
        mExplanation->appearMessage("StarPieceBank_Explain");
        MR::showLayout(this);
        MR::startAnim(this, "Appear", 0);
        mWithdrawButton->appear();
        mDepositButton->appear();
        mConfirmButton->appear();
        mCancelButton->appear();
    }
    if (MR::isAnimStopped(this, 0)) {
        MR::startStarPointerModeChooseYesNo(this);
        setNerve(&NrvStarPieceBank::StarPieceBankNrvSelect::sInstance);
    }
}

void StarPieceBank::exeSelect() {
    updatePointer();
    bool trade = false;
    if (mWithdrawButton->trySelectOnPointing() || mDepositButton->trySelectOnPointing()) trade = true;
    if (!trade) {
        if (isPadButtonUpOnly()) trade = mWithdrawButton->trySelectWithPad(isPadButtonUpOnly);
        else if (isPadButtonDownOnly()) trade = mDepositButton->trySelectWithPad(isPadButtonDownOnly);
    }
    if (trade) {
        setNerve(&NrvStarPieceBank::StarPieceBankNrvTradeStart::sInstance);
        return;
    }
    if (mConfirmButton->isPointingTrigger() || mCancelButton->isPointingTrigger() || mDepositButton->isPointingTrigger() || mWithdrawButton->isPointingTrigger()) {
        MR::startActionSound(this, "FocusItem", -1, -1);
    }
    if (mConfirmButton->trySelect()) {
        MR::startActionSound(this, "SelectYes", -1, -1);
        MR::startCSSound("CS_CLICK_CLOSE", 0, 0, 0, 1.0f);
        setNerve(&NrvStarPieceBank::StarPieceBankNrvConfirm::sInstance);
        return;
    }
    if (mCancelButton->trySelect()) {
        MR::startActionSound(this, "SelectNo", -1, -1);
        MR::startCSSound("CS_CLICK_CLOSE", 0, 0, 0, 1.0f);
        setNerve(&NrvStarPieceBank::StarPieceBankNrvCancel::sInstance);
    }
}

void StarPieceBank::exeTrade() {
    bool deposit = mDepositButton->isSelectedRepeat();
    bool withdraw = mWithdrawButton->isSelectedRepeat();
    if (!deposit && !withdraw) {
        if (isNerve(&NrvStarPieceBank::StarPieceBankNrvTrade::sInstance)) {
            MR::startActionSound(this, "CountEnd", -1, -1);
        }
        mWithdrawButton->trySelectOnPointing();
        mDepositButton->trySelectOnPointing();
        setNerve(&NrvStarPieceBank::StarPieceBankNrvSelect::sInstance);
        return;
    }
    s32 count = 1;
    if (isNerve(&NrvStarPieceBank::StarPieceBankNrvTradeStart::sInstance)) {
        if (MR::isStep(this, 20)) {
            setNerve(&NrvStarPieceBank::StarPieceBankNrvTrade::sInstance);
            return;
        }
        if (!MR::isFirstStep(this)) return;
        MR::tryRumblePadWeak(this, 0);
        if (deposit) MR::startActionSound(this, "DownCountOne", -1, -1);
        else MR::startActionSound(this, "UpCountOne", -1, -1);
    } else {
        if (MR::isGreaterStep(this, 60)) count = 6;
        if (MR::isGreaterStep(this, 150)) count = 11;
        if (MR::isGreaterStep(this, 240)) count = 71;
        if (!MR::isIntervalStep(this, 5)) return;
    }
    if (deposit && mBankedStarPieces > 0 && mStockedStarPieces < 9999) {
        if (count == 6) MR::startActionSound(this, "Count", -1, -1);
        else if (count == 11) MR::startActionSound(this, "CountFast", -1, -1);
        else if (count == 71) MR::startActionSound(this, "CountVeryFast", -1, -1);
        else MR::startActionSound(this, "Count", -1, -1);
        MR::tryRumblePadVeryWeak(this, 0);
        if (count >= mBankedStarPieces) count = mBankedStarPieces;
        if (count >= 9999 - mStockedStarPieces) count = 9999 - mStockedStarPieces;
        mBankedStarPieces -= count;
        mStockedStarPieces += count;
        MR::startPaneAnim(this, "StarPieceCounter", "DownCount", 0);
        updateCounters();
        return;
    }
    if (withdraw && mStockedStarPieces > 0 && mBankedStarPieces < 9999) {
        if (count == 6) MR::startActionSound(this, "Count", -1, -1);
        else if (count == 11) MR::startActionSound(this, "CountFast", -1, -1);
        else if (count == 71) MR::startActionSound(this, "CountVeryFast", -1, -1);
        else MR::startActionSound(this, "Count", -1, -1);
        MR::tryRumblePadVeryWeak(this, 0);
        if (count >= mStockedStarPieces) count = mStockedStarPieces;
        if (count >= 9999 - mBankedStarPieces) count = 9999 - mBankedStarPieces;
        mStockedStarPieces -= count;
        mBankedStarPieces += count;
        MR::startPaneAnim(this, "StarPieceCounter", "UpCount", 0);
        updateCounters();
    }
}

void StarPieceBank::exeDecided() {
    bool decided = false;
    if ((isNerve(&NrvStarPieceBank::StarPieceBankNrvConfirm::sInstance) && mConfirmButton->isDecidedWait()) ||
        (isNerve(&NrvStarPieceBank::StarPieceBankNrvCancel::sInstance) && mCancelButton->isDecidedWait())) {
        decided = true;
    }
    if (decided) {
        setNerve(&NrvStarPieceBank::StarPieceBankNrvDisappear::sInstance);
    }
}

void StarPieceBank::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "End", 0);
        mExplanation->disappear();
        MR::endStarPointerMode(this);
    }
    if (mConfirmButton->mIsSelected || mCancelButton->mIsSelected) {
        if (MR::isAnimStopped(this, 0)) {
            MR::decrementPauseBlurDelay();
            if (mConfirmButton->mIsSelected) {
                GameDataFunction::setBankedStarPieceNum(mBankedStarPieces);
                GameDataFunction::tryOnGameEventFlagStarPieceCounterStop(mStockedStarPieces - GameDataFunction::getStockedStarPieceNum());
            }
            MR::tryScreenToFrameCinemaFrame();
            kill();
        }
    }
}

void StarPieceBank::control() {
    mWithdrawButton->update();
    mDepositButton->update();
    mConfirmButton->update();
    mCancelButton->update();
    MR::setSeVolumeSetLevelForMenu();
}

void StarPieceBank::updateCounters() {
    mBankedStarPieces = MR::clamp(mBankedStarPieces, 0, 9999);
    mStockedStarPieces = MR::clamp(mStockedStarPieces, 0, 9999);
    MR::setTextBoxFormatRecursive(this, "ShaPieceUNum", L"%04d", mBankedStarPieces);
    MR::setTextBoxFormatRecursive(this, "ShaPieceDNum", L"%04d", mStockedStarPieces);
    bool noWithdraw = false;
    if (mStockedStarPieces <= 0 || mBankedStarPieces >= 9999) noWithdraw = true;
    if (noWithdraw) mWithdrawButton->mIsDisabled = true;
    else mWithdrawButton->mIsDisabled = false;
    bool noDeposit = false;
    if (mBankedStarPieces <= 0 || mStockedStarPieces >= 9999) noDeposit = true;
    if (noDeposit) mDepositButton->mIsDisabled = true;
    else mDepositButton->mIsDisabled = false;
}

void StarPieceBank::updatePointer() {
    if (StarPointerUtil::beginMovePositionRegistration(this, true)) {
        StarPointerUtil::addStarPointerMovePositionFromPane(this, "BoxLeft", StarPointerUtil::getDefaultButtonOffsetVec2());
        StarPointerUtil::addStarPointerMovePositionFromPane(this, "BoxRight", StarPointerUtil::getDefaultButtonOffsetVec2());
        StarPointerUtil::setFirstMovePosition("BoxLeft");
        StarPointerUtil::setLastMovePosition("BoxRight");
        StarPointerUtil::setConnectionMovePositionRight("BoxLeft", "BoxRight");
    }
    StarPointerUtil::refreshMovePositionsFromLayout(this);
    StarPointerUtil::endMovePositionRegistration(this);
}

StarPieceBank::~StarPieceBank() {}

void NrvStarPieceBank::StarPieceBankNrvWait::execute(Spine* spine) const {
    StarPieceBank* actor = reinterpret_cast<StarPieceBank*>(spine->mExecutor);
    if (MR::isStep(actor, 60)) {
        actor->setNerve(&StarPieceBankNrvAppear::sInstance);
    }
}
