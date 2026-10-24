#include "Game/Screen/StageResultInformer.hpp"
#include "Game/Screen/IconAButton.hpp"
#include "Game/NWC24/WiiMessageBoardMailSenderState.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/Util.hpp"
#include "Game/Screen/SimpleLayout.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/WorldMapResultCommand.hpp"
#include "Game/Scene/SceneObjHolder.hpp"

namespace {
    const char cMessagePowerStar[] = "System_ResultGetPowerStar";
    const char cMessageHiddenStar[] = "System_ResultGetHiddenStar";
    const char cMessageGreenStar[] = "System_ResultGetGreenStar";
    const char cMessageBronzeStar[] = "System_ResultGetBronzeStar";
    const char cMessageGrandStar[] = "System_ResultGetGrandStar";
    const char cMessageMedal[] = "System_ResultGetCometMedal";
}

extern const char cMessageCompleteGold[] = "System_ResultGalaxyCompleteGold";
extern const char cMessageCompleteSilver[] = "System_ResultGalaxyCompleteSilver";
extern const char cMessageGhostAppear[] = "System_ResultGhostAppear";

namespace {
    const char cMessageStarPiece[] = "System_ResultGetStarPiece";
    const char cMessageCoin[] = "System_ResultGetCoin";
    const char cMessageBestTime[] = "System_ResultUpdateBestTime";
    const char cPanePlayResult[] = "PlayResult";
    const char cPaneClearTime[] = "ClearTime";
    const char cNamePowerStarNum[] = "ShaStarNum";
    const char cNameStarPieceLeft[] = "ShaTPieceLNum";
    const char cNameStarPieceRight[] = "ShaTPieceRNum";
    const char cNameCoinLeft[] = "ShaTCoinLNum";
    const char cNameCoinRight[] = "ShaTCoinRNum";
    const char cMedalNum[] = "ShaMedalNum";
    const char cBestClearTime[] = "BClearTime";
    const char cCompleteMailFlag[] = "コンプリートメール送信";
}

namespace {
    bool tryDecideResult() {
        if (MR::testSystemPadTriggerDecide()) {
            MR::startSystemSE("SE_SY_TALK_OK");
            return true;
        }
        return false;
    }
}

DisplayInformationForResult::DisplayInformationForResult(StageResultInformer* host)
    : NerveExecutor("DisplayInformationForResult"), mHost(host), mMessageId(0), mReplaceString(0) {
    initNerve(&NrvDisplayInformationForResult::DisplayInformationForResultNrvIdle::sInstance);
}

char cBgmComplete[] = "BGM_JG_GALAXY_COMPLETE";
char cBgmNewGalaxy[] = "BGM_JG_NEW_GALAXY";

#pragma pool_data off
void DisplayInformationForResult::exeAppear() {
    if (MR::isFirstStep(this)) {
        if (MR::isEqualString(mMessageId, cMessageCompleteGold) || MR::isEqualString(mMessageId, cMessageCompleteSilver)) {
            MR::startSubBGM(cBgmComplete, false);
        } else {
            MR::startSubBGM(cBgmNewGalaxy, false);
        }
        MR::appearInformationMessageCenter(mMessageId, true);
        if (mReplaceString) MR::setInformationMessageReplaceString(mReplaceString, 0);
    }
    if (MR::isDeadInformationMessage()) setNerve(&NrvDisplayInformationForResult::DisplayInformationForResultNrvIdle::sInstance);
}
#pragma pool_data reset

StageResultInformer::StageResultInformer()
    : LayoutActor("リザルト画面", true), mInformationDisplayer(0), mIconAButton(0), mClearedStarPieceNum(0),
      mIsDecided(false), mOneUpLayout(0), mCoinOneUps(0), mResultPaneVisible(false) {
    mInformationDisplayer = new DisplayInformationForResult(this);
}

void StageResultInformer::init(const JMapInfoIter&) {
    MR::connectToSceneLayout(this);
    initLayoutManager("ResultWindow", 3);
    MR::createAndAddPaneCtrl(this, cPanePlayResult, 1);
    MR::createAndAddPaneCtrl(this, cPaneClearTime, 1);
    mIconAButton = MR::createAndSetupIconAButton(this, true, false);
    initNerve(&NrvStageResultInformer::StageResultInformerNrvAppearGetPowerStar::sInstance);
    initEffectKeeper(0, 0, 0);
    mOneUpLayout = new SimpleEffectLayout("1Upレイアウト", "OneUp", 1, 71);
    mOneUpLayout->initWithoutIter();
    MR::registerDemoSimpleCastAll(mOneUpLayout);
}

void StageResultInformer::appear() {
    LayoutActor::appear();
    mIconAButton->kill();
    MR::requestMovementOn(mIconAButton);
    MR::hideLayout(this);
    u32 bestTime = GameSequenceFunction::getResultBestTime();
    mResultPaneVisible = bestTime != 0;
    if (mResultPaneVisible) MR::updateClearTimeTextBox(this, cPaneClearTime, bestTime);
    else MR::hidePane(this, cPaneClearTime);
    MR::startAnimAndSetFrameAndStop(this, "BestTime", 0.0f, 2);
    setNerve(&NrvStageResultInformer::StageResultInformerNrvAppearGetPowerStar::sInstance);
}

void StageResultInformer::kill() {
    LayoutActor::kill();
    mIconAButton->kill();
}

// NONMATCHING: literal-pool and star-ID register allocation.
void StageResultInformer::exeAppearGetPowerStar() {
    s32 starId;
    if (MR::isFirstStep(this)) {
        MR::showLayout(this);
        hideCounterPanes();
        MR::showPane(this, "TotalStar");
        MR::startPaneAnim(this, cPanePlayResult, "Appear", 0);
        MR::startPaneAnim(this, cPaneClearTime, "Appear", 0);
        GalaxyStatusAccessor galaxy = MR::makeGalaxyStatusAccessor(GameSequenceFunction::getClearedStageName());
        starId = GameSequenceFunction::getClearedPowerStarId();
        const char* message;
        if (galaxy.isOwnBronzeStar(starId)) message = cMessageBronzeStar;
        else if (galaxy.isGrandStar(starId)) message = cMessageGrandStar;
        else if (galaxy.isHiddenStar(starId)) message = cMessageHiddenStar;
        else {
            bool green = galaxy.isGreenStar(starId);
            message = cMessagePowerStar;
            if (green) message = cMessageGreenStar;
        }
        MR::setTextBoxGameMessageRecursive(this, "Result", message);
        MR::startAnim(this, "Line", 1);
        MR::setAnimFrameAndStopAdjustTextHeight(this, "Result", 1);
        MR::setTextBoxArgStringRecursive(this, "Result", MR::getGalaxyNameOnCurrentLanguage(GameSequenceFunction::getClearedStageName()), 0);
        starId = GameSequenceFunction::getClearedPowerStarId();
        MR::setTextBoxArgStringRecursive(this, "Result", MR::getScenarioNameOnCurrentLanguage(GameSequenceFunction::getClearedStageName(), starId), 1);
        s32 stars;
        if (GameSequenceFunction::hasResultPowerStar()) stars = GameDataFunction::calcCurrentPowerStarNum() - 1;
        else stars = GameDataFunction::calcCurrentPowerStarNum();
        MR::setTextBoxNumberRecursive(this, cNamePowerStarNum, stars);
        MR::startSubBGM("BGM_JG_RETURN_HOME", false);
    }
    MR::setNerveAtAnimStopped(this, &NrvStageResultInformer::StageResultInformerNrvWaitBeforeCountUpPowerStar::sInstance, 0);
}

void StageResultInformer::exeCountUpPowerStar() {
    if (MR::isFirstStep(this)) {
        MR::setTextBoxNumberRecursive(this, cNamePowerStarNum, GameDataFunction::calcCurrentPowerStarNum());
        MR::startAnim(this, "Flash", 0);
        MR::startSystemSE("SE_SY_RESULT_POW_STAR_INC");
        MR::emitEffect(this, "ResultStarCounter");
    }

    if (MR::testSystemPadTriggerDecide()) {
        mIsDecided = true;
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

void StageResultInformer::exeDisappearGetPowerStar() {
    if (MR::isFirstStep(this)) {
        endResultPane();
        mIconAButton->term();
    }
    if (MR::isPaneAnimStopped(this, cPanePlayResult, 0)) decideNextNerve();
}

void StageResultInformer::exeAppearGetCometMedal() {
    if (MR::isFirstStep(this)) {
        MR::showLayout(this);
        hideCounterPanes();
        MR::showPane(this, "TotalMedal");
        MR::startPaneAnim(this, cPanePlayResult, "Appear", 0);
        MR::setTextBoxGameMessageRecursive(this, "Result", cMessageMedal);
        MR::startAnim(this, "Line", 1);
        MR::setAnimFrameAndStopAdjustTextHeight(this, "Result", 1);
        MR::setTextBoxNumberRecursive(this, cMedalNum, GameDataFunction::calcTicoCoinNum());
        MR::startSubBGM("BGM_JG_HIGH_SCORE", false);
    }
    MR::setNerveAtPaneAnimStopped(this, cPanePlayResult, &NrvStageResultInformer::StageResultInformerNrvWaitBeforeCountUpCometMedal::sInstance, 0);
}

void StageResultInformer::exeCountUpGetCometMedal() {
    MR::isFirstStep(this);
    if (MR::testSystemPadTriggerDecide()) {
        mIsDecided = true;
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

void StageResultInformer::exeDisappearGetCometMedal() {
    if (MR::isFirstStep(this)) {
        endResultPane();
        mIconAButton->term();
    }
    if (MR::isPaneAnimStopped(this, cPanePlayResult, 0)) decideNextNerve();
}

#pragma pool_data off
void StageResultInformer::exeComplete() {
    GalaxyStatusAccessor galaxy = MR::makeGalaxyStatusAccessor(GameSequenceFunction::getClearedStageName());
    bool complete = galaxy.isCompletedNoGreen();
    const char* message = cMessageCompleteSilver;
    if (complete) message = cMessageCompleteGold;
    tryShowAndKeyWaitInformationWindow(message, 0);
}
#pragma pool_data reset

void StageResultInformer::exeAppearGetStarPiece() {
    if (MR::isFirstStep(this)) {
        hideCounterPanes();
        MR::showPane(this, "TotalPiece");
        MR::startPaneAnim(this, cPanePlayResult, "Appear", 0);
        MR::setTextBoxGameMessageRecursive(this, "Result", cMessageStarPiece);
        MR::startAnim(this, "Line", 1);
        MR::setAnimFrameAndStopAdjustTextHeight(this, "Result", 1);
        mClearedStarPieceNum = GameSequenceFunction::getClearedStarPieceNum();
        updateStarPieceCount();
    }
    MR::setNerveAtPaneAnimStopped(this, cPanePlayResult, &NrvStageResultInformer::StageResultInformerNrvDisplayGetStarPiece::sInstance, 0);
}

void StageResultInformer::exeDisplayGetStarPiece() {
    if (MR::isFirstStep(this)) {
        mIconAButton->openWithoutMessage();
        MR::emitEffect(this, "ResultPieceCounter");
    }
    bool finished = false;
    if (mClearedStarPieceNum > 0) {
        --mClearedStarPieceNum;
        if (mClearedStarPieceNum > 0) {
            if (mClearedStarPieceNum % 2 == 0) MR::startSystemSE("SE_SY_STAR_PIECE_SUM_UP");
        } else {
            MR::startSystemSE("SE_SY_STAR_PIECE_SUM_UP_END");
        }
    }
    if (mClearedStarPieceNum == 0) {
        finished = true;
        MR::deleteEffect(this, "ResultPieceCounter");
    }
    updateStarPieceCount();
    if (MR::isAnimStopped(this, 0) && !finished) MR::startAnim(this, "Flash", 0);
    if (tryDecideResult()) {
        if (finished) setNerve(&NrvStageResultInformer::StageResultInformerNrvDisappearGetStarPiece::sInstance);
        else setNerve(&NrvStageResultInformer::StageResultInformerNrvWaitBeforeDisappearGetStarPiece::sInstance);
    }
}

void StageResultInformer::exeWaitBeforeDisappearGetStarPiece() {
    if (MR::isFirstStep(this)) {
        mIconAButton->term();
        MR::startSystemSE("SE_SY_STAR_PIECE_SUM_UP_END");
        mClearedStarPieceNum = 0;
        updateStarPieceCount();
        MR::deleteEffect(this, "ResultPieceCounter");
    }
    MR::setNerveAtStep(this, &NrvStageResultInformer::StageResultInformerNrvDisappearGetStarPiece::sInstance, 45);
}

void StageResultInformer::exeDisappearGetStarPiece() {
    if (MR::isFirstStep(this)) {
        endResultPane();
        if (mIconAButton->isOpen()) mIconAButton->term();
        MR::forceSyncStarPieceCounter();
    }
    if (MR::isPaneAnimStopped(this, cPanePlayResult, 0)) decideNextNerve();
}

void StageResultInformer::exeAppearGetCoin() {
    if (MR::isFirstStep(this)) {
        hideCounterPanes();
        MR::showPane(this, "TotalCoin");
        MR::startPaneAnim(this, cPanePlayResult, "Appear", 0);
        MR::setTextBoxGameMessageRecursive(this, "Result", cMessageCoin);
        MR::startAnim(this, "Line", 1);
        MR::setAnimFrameAndStopAdjustTextHeight(this, "Result", 1);
        mClearedStarPieceNum = GameSequenceFunction::getClearedCoinNum();
        updateCoinCount();
        mCoinOneUps = 0;
    }
    if (MR::isPaneAnimStopped(this, cPanePlayResult, 0)) {
        mIconAButton->openWithoutMessage();
        setNerve(&NrvStageResultInformer::StageResultInformerNrvDisplayGetCoin::sInstance);
    }
}

void StageResultInformer::exeDisplayGetCoin() {
    if (MR::isFirstStep(this)) MR::emitEffect(this, "ResultCoinCounter");
    --mClearedStarPieceNum;
    if (mClearedStarPieceNum % 2 == 0) MR::startSystemSE("SE_SY_COIN_COUNTUP");
    if (MR::isAnimStopped(this, 0)) MR::startAnim(this, "Flash", 0);
    updateCoinCount();
    if (tryDecideResult()) {
        mIsDecided = true;
        mIconAButton->term();
        if (mCoinOneUps < GameSequenceFunction::getResultCoinOneUpCount()) setNerve(&NrvStageResultInformer::StageResultInformerNrvWaitBeforeDisappearGetCoin::sInstance);
        else {
            MR::startSystemSE("SE_SY_COIN_COUNTUP_END");
            setNerve(&NrvStageResultInformer::StageResultInformerNrvCoinCountEnd::sInstance);
        }
        mClearedStarPieceNum = 0;
        updateCoinCount();
        return;
    }
    if (shouldShowCoinOneUp()) {
        setNerve(&NrvStageResultInformer::StageResultInformerNrvWaitBeforeDisappearGetCoin::sInstance);
        return;
    }
    if (mClearedStarPieceNum == 0) {
        MR::startSystemSE("SE_SY_COIN");
        setNerve(&NrvStageResultInformer::StageResultInformerNrvCoinCountWait::sInstance);
    }
}

void StageResultInformer::exeWaitBeforeDisappearGetCoin() {
    if (MR::isFirstStep(this)) {
        MR::setLayoutPosAtPaneTrans(mOneUpLayout, this, "EffPosTCoin");
        MR::setTextBoxArgNumberRecursive(mOneUpLayout, "OneUp", 1, 0);
        mOneUpLayout->appear();
        MR::startAnim(mOneUpLayout, "Appear", 0);
        MR::emitEffect(mOneUpLayout, "OneUpGet");
        MR::startSystemSE("SE_SY_1UP");
    }
    if (!mIsDecided && tryDecideResult()) {
        mIsDecided = true;
        mIconAButton->term();
        mClearedStarPieceNum = 0;
        updateCoinCount();
    }
    if (MR::isAnimStopped(mOneUpLayout, 0)) {
        ++mCoinOneUps;
        if (mIsDecided && mCoinOneUps < GameSequenceFunction::getResultCoinOneUpCount()) {
            setNerve(&NrvStageResultInformer::StageResultInformerNrvWaitBeforeDisappearGetCoin::sInstance);
            return;
        }
        s32 current = GameDataFunction::getStockedCoinNum() - mClearedStarPieceNum;
        if (current < GameDataFunction::getStockedCoinNum()) {
            setNerve(&NrvStageResultInformer::StageResultInformerNrvDisplayGetCoin::sInstance);
            return;
        }
        if (mIsDecided) {
            setNerve(&NrvStageResultInformer::StageResultInformerNrvDisappearGetCoin::sInstance);
            return;
        }
        setNerve(&NrvStageResultInformer::StageResultInformerNrvCoinCountWait::sInstance);
    }
}

void StageResultInformer::exeShowGalaxyMap() {
    if (MR::isFirstStep(this)) updateCoinCount();
    if (tryDecideResult()) setNerve(&NrvStageResultInformer::StageResultInformerNrvDisappearGetCoin::sInstance);
}

void StageResultInformer::exeDisappearGetCoin() {
    if (MR::isFirstStep(this)) {
        endResultPane();
        if (mIconAButton->isOpen()) mIconAButton->term();
        mOneUpLayout->kill();
        MR::forceSyncCoinCounter();
    }
    if (MR::isPaneAnimStopped(this, cPanePlayResult, 0)) decideNextNerve();
}

void StageResultInformer::exeAppearBestTime() {
    if (MR::isFirstStep(this)) {
        MR::showLayout(this);
        MR::startPaneAnim(this, cPanePlayResult, "Appear", 0);
        MR::startAnim(this, "BestTime", 2);
        hideCounterPanes();
        MR::showPane(this, "BClearTime");
        MR::setTextBoxGameMessageRecursive(this, "Result", cMessageBestTime);
        MR::startAnim(this, "Line", 1);
        MR::setAnimFrameAndStopAdjustTextHeight(this, "Result", 1);
        MR::updateClearTimeTextBox(this, cBestClearTime, GameSequenceFunction::getResultBestTime());
        MR::startSubBGM("BGM_SMG2_JG_TIME", false);
    }
    MR::setNerveAtPaneAnimStopped(this, cPanePlayResult, &NrvStageResultInformer::StageResultInformerNrvWaitBeforeDisplayBestTime::sInstance, 0);
}

void StageResultInformer::exeDisplayBestTime() {
    if (MR::isFirstStep(this)) {
        mIconAButton->openWithoutMessage();
    }
    if (tryWaitSystemPadTriggerDecide(&NrvStageResultInformer::StageResultInformerNrvDisappearBestTime::sInstance)) {
        return;
    }
}

void StageResultInformer::exeDisappearBestTime() {
    if (MR::isFirstStep(this)) {
        endResultPane();
        MR::startAnimAndSetFrameAndStop(this, "BestTime", 0.0f, 2);
        mIconAButton->term();
    }
    if (MR::isPaneAnimStopped(this, cPanePlayResult, 0)) decideNextNerve();
}

void StageResultInformer::exeHideClearTime() {
    if (MR::isFirstStep(this)) {
        mResultPaneVisible = false;
        MR::startPaneAnim(this, cPaneClearTime, "End", 0);
    }
    if (MR::isPaneAnimStopped(this, cPaneClearTime, 0)) decideNextNerve();
}

namespace {
    char cCompleteMailMessage[] = "WiiMessage_AllComplete";
    char cCompleteMailImage[] = "AllCompleteImage.bin";
}

#pragma pool_data off
void StageResultInformer::exeSendCompleteMail() {
    if (MR::isFirstStep(this)) MR::requestWiiMessageBoardMailSend(cCompleteMailMessage, cCompleteMailImage, cCompleteMailFlag);
    if (MR::updateWiiMessageBoardMailSenderState()) decideNextNerve();
}
#pragma pool_data reset

void StageResultInformer::exeMapStart() {
    MR::getSceneObj<WorldMapResultCommand>(0x34)->updateController();
    if (MR::getSceneObj<WorldMapResultCommand>(0x34)->isStateDead()) decideNextNerve();
}

void StageResultInformer::exeMapEnd() {
    MR::getSceneObj<WorldMapResultCommand>(0x34)->updateController();
    if (MR::getSceneObj<WorldMapResultCommand>(0x34)->isStateDead()) decideNextNerve();
}

// NONMATCHING: cumulative state flags use different callee-saved registers.
const Nerve* StageResultInformer::getNextNerve() const {
    bool afterStar = isNerve(&NrvStageResultInformer::StageResultInformerNrvDisappearGetPowerStar::sInstance);
    bool afterMedal = false;
    if (afterStar || isNerve(&NrvStageResultInformer::StageResultInformerNrvDisappearGetCometMedal::sInstance)) afterMedal = true;
    bool afterComplete = false;
    if (afterMedal || isNerve(&NrvStageResultInformer::StageResultInformerNrvComplete::sInstance)) afterComplete = true;
    bool afterGhost = false;
    if (afterComplete || isNerve(&NrvStageResultInformer::StageResultInformerNrvGhostAppear::sInstance)) afterGhost = true;
    bool afterPiece = false;
    if (afterGhost || isNerve(&NrvStageResultInformer::StageResultInformerNrvDisappearGetStarPiece::sInstance)) afterPiece = true;
    bool afterCoin = false;
    if (afterPiece || isNerve(&NrvStageResultInformer::StageResultInformerNrvDisappearGetCoin::sInstance)) afterCoin = true;
    bool afterBestTime = false;
    if (afterCoin || isNerve(&NrvStageResultInformer::StageResultInformerNrvDisappearBestTime::sInstance)) afterBestTime = true;
    bool afterClearTime = false;
    if (afterBestTime || isNerve(&NrvStageResultInformer::StageResultInformerNrvHideClearTime::sInstance)) afterClearTime = true;
    bool afterMail = false;
    if (afterClearTime || isNerve(&NrvStageResultInformer::StageResultInformerNrvSendCompleteMail::sInstance)) afterMail = true;
    bool afterMapStart = false;
    if (afterMail || isNerve(&NrvStageResultInformer::StageResultInformerNrvMapStart::sInstance)) afterMapStart = true;
    bool afterRoute = false;
    if (afterMapStart || isNerve(&NrvStageResultInformer::StageResultInformerNrvRouteOpen::sInstance)) afterRoute = true;
    bool afterComet = false;
    if (afterRoute || isNerve(&NrvStageResultInformer::StageResultInformerNrvCometAppear::sInstance)) afterComet = true;
    if (afterStar && GameSequenceFunction::isResultNotifyTicoCoin()) return &NrvStageResultInformer::StageResultInformerNrvAppearGetCometMedal::sInstance;
    if (afterMedal && isCompleted()) return &NrvStageResultInformer::StageResultInformerNrvComplete::sInstance;
    if (afterComplete && GameSequenceFunction::isResultUnlockGhostLuigi()) return &NrvStageResultInformer::StageResultInformerNrvGhostAppear::sInstance;
    if (afterGhost && GameSequenceFunction::getClearedStarPieceNum() > 0) return &NrvStageResultInformer::StageResultInformerNrvAppearGetStarPiece::sInstance;
    if (afterPiece && GameSequenceFunction::getClearedCoinNum() > 0) return &NrvStageResultInformer::StageResultInformerNrvAppearGetCoin::sInstance;
    if (afterCoin && GameSequenceFunction::isResultNewBestTime()) return &NrvStageResultInformer::StageResultInformerNrvAppearBestTime::sInstance;
    if (afterBestTime && mResultPaneVisible) return &NrvStageResultInformer::StageResultInformerNrvHideClearTime::sInstance;
    if (afterClearTime) {
        bool sendMail;
        if (GameDataFunction::isOnGameEventFlag(cCompleteMailFlag)) sendMail = false;
        else {
            s32 maximum = GameDataFunction::getPowerStarNumMax();
            sendMail = maximum <= GameDataFunction::calcCurrentPowerStarNum();
        }
        if (sendMail) return &NrvStageResultInformer::StageResultInformerNrvSendCompleteMail::sInstance;
    }
    if (!MR::isStageMarioFaceShipNormal()) return 0;
    if (afterMail && MR::getSceneObj<WorldMapResultCommand>(0x34)->tryStartResultMapStart()) return &NrvStageResultInformer::StageResultInformerNrvMapStart::sInstance;
    if (afterMapStart && MR::getSceneObj<WorldMapResultCommand>(0x34)->tryRouteOpen()) return &NrvStageResultInformer::StageResultInformerNrvRouteOpen::sInstance;
    if (afterRoute && MR::getSceneObj<WorldMapResultCommand>(0x34)->tryCometAppear()) return &NrvStageResultInformer::StageResultInformerNrvCometAppear::sInstance;
    if (afterComet && MR::getSceneObj<WorldMapResultCommand>(0x34)->tryStartResultMapEnd()) return &NrvStageResultInformer::StageResultInformerNrvMapEnd::sInstance;
    return 0;
}

void StageResultInformer::decideNextNerve() {
    mIsDecided = false;
    const Nerve* next = getNextNerve();
    if (next) setNerve(next);
    else kill();
}

bool StageResultInformer::tryWaitSystemPadTriggerDecide(const Nerve* pNerve) {
    if (MR::testSystemPadTriggerDecide() || mIsDecided) {
        mIsDecided = false;

        setNerve(pNerve);
        MR::startSystemSE("SE_SY_TALK_OK");

        return true;
    }

    return false;
}

bool StageResultInformer::tryWaitIntervalBeforeKeyWait(const Nerve* pNerve, int step) {
    if (MR::testSystemPadTriggerDecide()) {
        mIsDecided = true;
    }

    if (MR::isStep(this, step)) {
        setNerve(pNerve);

        return true;
    }

    return false;
}

bool StageResultInformer::tryShowAndKeyWaitInformationWindow(const char* message, const wchar_t* replace) {
    if (MR::isFirstStep(this)) {
        DisplayInformationForResult* info = mInformationDisplayer;
        info->mMessageId = message;
        info->mReplaceString = replace;
        info->setNerve(&NrvDisplayInformationForResult::DisplayInformationForResultNrvAppear::sInstance);
    }
    mInformationDisplayer->updateNerve();
    bool notIdle = !mInformationDisplayer->isNerve(&NrvDisplayInformationForResult::DisplayInformationForResultNrvIdle::sInstance);
    if (!notIdle) {
        decideNextNerve();
        return true;
    }
    return false;
}

void StageResultInformer::updateStarPieceCount() {
    s32 count = mClearedStarPieceNum;
    MR::setTextBoxNumberRecursive(this, cNameStarPieceLeft, count);
    MR::setTextBoxNumberRecursive(this, cNameStarPieceRight, GameDataFunction::getStockedStarPieceNum() - mClearedStarPieceNum);
}

void StageResultInformer::updateCoinCount() {
    s32 count = mClearedStarPieceNum;
    MR::setTextBoxNumberRecursive(this, cNameCoinLeft, count);
    MR::setTextBoxNumberRecursive(this, cNameCoinRight, GameDataFunction::getStockedCoinNum() - mClearedStarPieceNum);
}

bool StageResultInformer::shouldShowCoinOneUp() {
    if (GameSequenceFunction::getResultCoinOneUpCount() <= mCoinOneUps) return false;
    if ((GameDataFunction::getStockedCoinNum() - mClearedStarPieceNum) % 100 > 0) return false;
    s32 stocked = GameDataFunction::getStockedCoinNum();
    stocked -= GameSequenceFunction::getResultCoinOneUpCount() * 100;
    return stocked < GameDataFunction::getStockedCoinNum() - mClearedStarPieceNum;
}

void StageResultInformer::hideCounterPanes() {
    MR::hidePane(this, "TotalStar");
    MR::hidePane(this, "TotalMedal");
    MR::hidePane(this, "TotalPiece");
    MR::hidePane(this, "TotalCoin");
    MR::hidePane(this, "BClearTime");
}

void StageResultInformer::endResultPane() {
    MR::startPaneAnim(this, cPanePlayResult, "End", 0);
    const Nerve* next = getNextNerve();
    if (next == &NrvStageResultInformer::StageResultInformerNrvAppearGetCometMedal::sInstance ||
        next == &NrvStageResultInformer::StageResultInformerNrvComplete::sInstance ||
        next == &NrvStageResultInformer::StageResultInformerNrvGhostAppear::sInstance ||
        next == &NrvStageResultInformer::StageResultInformerNrvAppearGetStarPiece::sInstance ||
        next == &NrvStageResultInformer::StageResultInformerNrvAppearGetCoin::sInstance ||
        next == &NrvStageResultInformer::StageResultInformerNrvAppearBestTime::sInstance) return;
    {
        MR::startPaneAnim(this, cPaneClearTime, "End", 0);
        mResultPaneVisible = false;
    }
}

bool StageResultInformer::isCompleted() const {
    GalaxyStatusAccessor galaxy = MR::makeGalaxyStatusAccessor(GameSequenceFunction::getClearedStageName());
    if (!GameSequenceFunction::hasResultPowerStar() && !GameSequenceFunction::isResultNotifyTicoCoin() && !GameSequenceFunction::isResultStarNotGreen()) return false;
    bool complete;
    if (galaxy.isCompletedNoGreen()) complete = true;
    else {
        complete = false;
        if (galaxy.isCompletedWithGreen() && !galaxy.isGreenStar(GameSequenceFunction::getClearedPowerStarId())) complete = true;
    }
    return complete;
}

StageResultInformer::~StageResultInformer() {}

namespace NrvStageResultInformer {
    StageResultInformerNrvAppearGetPowerStar StageResultInformerNrvAppearGetPowerStar::sInstance;
    StageResultInformerNrvWaitBeforeCountUpPowerStar StageResultInformerNrvWaitBeforeCountUpPowerStar::sInstance;
    StageResultInformerNrvCountUpPowerStar StageResultInformerNrvCountUpPowerStar::sInstance;
    StageResultInformerNrvDisplayGetPowerStar StageResultInformerNrvDisplayGetPowerStar::sInstance;
    StageResultInformerNrvDisappearGetPowerStar StageResultInformerNrvDisappearGetPowerStar::sInstance;
    StageResultInformerNrvAppearGetCometMedal StageResultInformerNrvAppearGetCometMedal::sInstance;
    StageResultInformerNrvWaitBeforeCountUpCometMedal StageResultInformerNrvWaitBeforeCountUpCometMedal::sInstance;
    StageResultInformerNrvCountUpGetCometMedal StageResultInformerNrvCountUpGetCometMedal::sInstance;
    StageResultInformerNrvDisplayGetCometMedal StageResultInformerNrvDisplayGetCometMedal::sInstance;
    StageResultInformerNrvDisappearGetCometMedal StageResultInformerNrvDisappearGetCometMedal::sInstance;
    StageResultInformerNrvComplete StageResultInformerNrvComplete::sInstance;
    StageResultInformerNrvGhostAppear StageResultInformerNrvGhostAppear::sInstance;
    StageResultInformerNrvAppearGetStarPiece StageResultInformerNrvAppearGetStarPiece::sInstance;
    StageResultInformerNrvDisplayGetStarPiece StageResultInformerNrvDisplayGetStarPiece::sInstance;
    StageResultInformerNrvWaitBeforeDisappearGetStarPiece StageResultInformerNrvWaitBeforeDisappearGetStarPiece::sInstance;
    StageResultInformerNrvDisappearGetStarPiece StageResultInformerNrvDisappearGetStarPiece::sInstance;
    StageResultInformerNrvAppearGetCoin StageResultInformerNrvAppearGetCoin::sInstance;
    StageResultInformerNrvDisplayGetCoin StageResultInformerNrvDisplayGetCoin::sInstance;
    StageResultInformerNrvWaitBeforeDisappearGetCoin StageResultInformerNrvWaitBeforeDisappearGetCoin::sInstance;
    StageResultInformerNrvCoinCountEnd StageResultInformerNrvCoinCountEnd::sInstance;
    StageResultInformerNrvCoinCountWait StageResultInformerNrvCoinCountWait::sInstance;
    StageResultInformerNrvDisappearGetCoin StageResultInformerNrvDisappearGetCoin::sInstance;
    StageResultInformerNrvAppearBestTime StageResultInformerNrvAppearBestTime::sInstance;
    StageResultInformerNrvWaitBeforeDisplayBestTime StageResultInformerNrvWaitBeforeDisplayBestTime::sInstance;
    StageResultInformerNrvDisplayBestTime StageResultInformerNrvDisplayBestTime::sInstance;
    StageResultInformerNrvDisappearBestTime StageResultInformerNrvDisappearBestTime::sInstance;
    StageResultInformerNrvHideClearTime StageResultInformerNrvHideClearTime::sInstance;
    StageResultInformerNrvSendCompleteMail StageResultInformerNrvSendCompleteMail::sInstance;
    StageResultInformerNrvMapStart StageResultInformerNrvMapStart::sInstance;
    StageResultInformerNrvRouteOpen StageResultInformerNrvRouteOpen::sInstance;
    StageResultInformerNrvCometAppear StageResultInformerNrvCometAppear::sInstance;
    StageResultInformerNrvMapEnd StageResultInformerNrvMapEnd::sInstance;
}

namespace {
    const TVec2f cTextSize21x23(21.0f, 23.0f);
    const TVec2f cTextSize21x24(21.0f, 24.0f);
}

namespace NrvDisplayInformationForResult {
    DisplayInformationForResultNrvAppear DisplayInformationForResultNrvAppear::sInstance;
    DisplayInformationForResultNrvIdle DisplayInformationForResultNrvIdle::sInstance;
}
