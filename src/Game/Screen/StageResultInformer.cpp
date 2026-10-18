#include "Game/Screen/StageResultInformer.hpp"
#include "Game/Screen/IconAButton.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Util.hpp"

namespace {
    const char cNamePowerStarNum[] = "ShaStarNum";
}

DisplayInformationForResult::DisplayInformationForResult(StageResultInformer* host)
    : NerveExecutor("DisplayInformationForResult"), mHost(host), mMessageId(0), mBgmName(0) {
    initNerve(&NrvDisplayInformationForResult::DisplayInformationForResultNrvIdle::sInstance);
}

StageResultInformer::StageResultInformer()
    : LayoutActor("リザルト通知", true), mInformationDisplayer(0), mIconAButton(0), mClearedStarPieceNum(0),
      _30(false), mPreviousPowerStarNum(0), mPreviousCometMedalNum(0), mResultPaneVisible(false) {
    mInformationDisplayer = new DisplayInformationForResult(this);
}

void StageResultInformer::kill() {
    LayoutActor::kill();
    mIconAButton->kill();
}

void StageResultInformer::exeCountUpPowerStar() {
    if (MR::isFirstStep(this)) {
        MR::setTextBoxNumberRecursive(this, cNamePowerStarNum, GameDataFunction::calcCurrentPowerStarNum());
        MR::startAnim(this, "Flash", 0);
        MR::startSystemSE("SE_SY_RESULT_POW_STAR_INC");
        MR::emitEffect(this, "ResultStarCounter");
    }

    if (MR::testSystemPadTriggerDecide()) {
        _30 = true;
    }

    MR::setNerveAtAnimStopped(this, &NrvStageResultInformer::StageResultInformerNrvDisplayGetPowerStar::sInstance, 0);
}

void StageResultInformer::exeDisplayGetPowerStar() {
    if (MR::isFirstStep(this)) {
        mIconAButton->openWithoutMessage();
    }

    if (tryWaitSystemPadTriggerDecide(&NrvStageResultInformer::StageResultInformerNrvDisappearGetPowerStar::sInstance)) {
        return;
    }
}

bool StageResultInformer::tryWaitSystemPadTriggerDecide(const Nerve* pNerve) {
    if (MR::testSystemPadTriggerDecide() || _30) {
        _30 = false;

        setNerve(pNerve);
        MR::startSystemSE("SE_SY_TALK_OK");

        return true;
    }

    return false;
}

bool StageResultInformer::tryWaitIntervalBeforeKeyWait(const Nerve* pNerve, int step) {
    if (MR::testSystemPadTriggerDecide()) {
        _30 = true;
    }

    if (MR::isStep(this, step)) {
        setNerve(pNerve);

        return true;
    }

    return false;
}

void StageResultInformer::updateStarPieceCount() {
    s32 count = mClearedStarPieceNum;
    MR::setTextBoxNumberRecursive(this, "ShaTPieceLNum", count);
    MR::setTextBoxNumberRecursive(this, "ShaTPieceRNum", GameDataFunction::getStockedStarPieceNum() - mClearedStarPieceNum);
}

void StageResultInformer::updateCoinCount() {
    s32 count = mClearedStarPieceNum;
    MR::setTextBoxNumberRecursive(this, "ShaTCoinLNum", count);
    MR::setTextBoxNumberRecursive(this, "ShaTCoinRNum", GameDataFunction::getStockedCoinNum() - mClearedStarPieceNum);
}

void StageResultInformer::hideCounterPanes() {
    MR::hidePane(this, "TotalStar");
    MR::hidePane(this, "TotalMedal");
    MR::hidePane(this, "TotalPiece");
    MR::hidePane(this, "TotalCoin");
    MR::hidePane(this, "BClearTime");
}

void StageResultInformer::exeCountUpGetCometMedal() {
    MR::isFirstStep(this);
    if (MR::testSystemPadTriggerDecide()) {
        _30 = true;
    }
    MR::setNerveAtAnimStopped(this, &NrvStageResultInformer::StageResultInformerNrvDisplayGetCometMedal::sInstance, 0);
}

void StageResultInformer::exeDisplayGetCometMedal() {
    if (MR::isFirstStep(this)) {
        mIconAButton->openWithoutMessage();
    }
    if (tryWaitSystemPadTriggerDecide(&NrvStageResultInformer::StageResultInformerNrvDisappearGetCometMedal::sInstance)) {
        return;
    }
}

void StageResultInformer::exeDisplayBestTime() {
    if (MR::isFirstStep(this)) {
        mIconAButton->openWithoutMessage();
    }
    if (tryWaitSystemPadTriggerDecide(&NrvStageResultInformer::StageResultInformerNrvDisappearBestTime::sInstance)) {
        return;
    }
}


StageResultInformer::~StageResultInformer() {}
