#include "Game/Map/RaceManager.hpp"
#include "Game/Util.hpp"
#include "Game/NPC/EventDirector.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Scene/ScenePlayingResult.hpp"
#include "Game/Screen/InformationObserver.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameDataConst.hpp"
#include "Game/System/GameDataFunction.hpp"

#include "Game/System/GameDataGalaxyStorage.hpp"
#include "Game/System/GameEventFlag.hpp"
#include "Game/System/GameEventFlagTable.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSequence.hpp"
#include "Game/System/PlayResultInStageHolder.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/NPC/PostmanEvent.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <cstdio>


namespace GameDataFunction {
    bool isOnGameEventFlag(const char*);
    void onGameEventFlag(const char*);
};  // namespace GameDataFunction

namespace {
};  // namespace

namespace MR {















    bool isOnGameEventFlagNormalEnding() {
        return GameDataFunction::isOnGameEventFlag("ノーマルエンディング実行");
    }

    bool isOnGameEventFlag120PowerStarsEnding() {
        return GameDataFunction::isOnGameEventFlag("スター120個エンディング実行");
    }

    bool isPreventDisplayInformationMessage() {
        if (isStageStaffRoll()) {
            return true;
        }

        return GameSequenceFunction::isPadMode2();
    }

    void incPlayerAndTryScenarioMissNum(bool isGameOver) {
        GameDataFunction::incPlayerMissNum();

        if (!isGameOver && isProductGameScene() && !isStageMarioFaceShip()) {
            if (isInAreaObj("DreamerDeadCountArea", *getPlayerPos())) {
                s32 scenarioNo = getCurrentScenarioNo();
                GameDataFunction::incScenarioMissNum(getCurrentStageName(), scenarioNo);
            }
        }

        if (!GameSequenceFunction::isResultMiss()) {
            GameDataFunction::updateLuigiIntrusionCounter();
        }
    }

    s32 getPlayerLeft() {
        return GameDataFunction::getPlayerLeft();
    }

    void addPlayerLeft(int lives) {
        requestOneUpBoard(lives);

        if (!GameSequenceFunction::isPadModeDreamer()) {
            GameDataFunction::addPlayerLeft(lives);
        }
    }

    void decPlayerLeft() {
        if (!GameSequenceFunction::isPadModeDreamer() && !isStageMarioFaceShip()) {
            GameDataFunction::addPlayerLeft(-1);
            incPlayerAndTryScenarioMissNum(false);
        }
    }

    void incPlayerGameOverNum() {
        GameDataFunction::incPlayerGameOverNum();
        incPlayerAndTryScenarioMissNum(true);
    }

    bool isPlayerLuigi() {
        return GameDataFunction::isLuigiPlayer();
    }

    s32 getStarPieceNum() {
        bool isStockedPlace = isStageMarioFaceShipOrWorldMap() || isStageNoPauseMenuStars();
        if (isStockedPlace) {
            return GameDataFunction::getStockedStarPieceNum();
        }

        return GameSequenceFunction::getPlayResultInStageHolder()->getClearedStarPieceNum();
    }

    void addStarPiece(int num) {
        if (num > 0) {
            ScoreAttackFunction::score10PointsMultiply(num);
        }

        if (!GameSequenceFunction::isPadModeDreamer()) {
            bool isStockedPlace = isStageMarioFaceShipOrWorldMap() || isStageNoPauseMenuStars();
            if (isStockedPlace) {
                GameDataFunction::tryOnGameEventFlagStarPieceCounterStop(num);
            } else if (GameSequenceFunction::getPlayResultInStageHolder()->addStarPieceNum(num)) {
                requestOneUpBoard(1);

                if (!GameSequenceFunction::isPadModeDreamer()) {
                    GameDataFunction::addPlayerLeft(1);
                }
            }
        }
    }

    void incCoin(int num, LiveActor* pHost) {
        ScoreAttackFunction::score100PointsMultiply(pHost, num);

        if (!GameSequenceFunction::isPadModeDreamer()) {
            bool isStockedPlace = isStageMarioFaceShipOrWorldMap() || isStageNoPauseMenuStars();
            if (isStockedPlace) {
                GameDataFunction::tryOnGameEventFlagCoinCounterStop(num);
            } else {
                GameSequenceFunction::getPlayResultInStageHolder()->addCoinNum(num);
            }

            incPlayerLife(num);
        }
    }

    void decCoin(int num) {
        bool isStockedPlace = isStageMarioFaceShipOrWorldMap() || isStageNoPauseMenuStars();
        if (isStockedPlace) {
            GameDataFunction::tryOnGameEventFlagCoinCounterStop(-num);
        } else {
            GameSequenceFunction::getPlayResultInStageHolder()->addCoinNum(-num);
        }
    }

    s32 getCoinNum() {
        bool isStockedPlace = isStageMarioFaceShipOrWorldMap() || isStageNoPauseMenuStars();
        if (isStockedPlace) {
            return GameDataFunction::getStockedCoinNum();
        }

        return GameSequenceFunction::getPlayResultInStageHolder()->getClearedCoinNum();
    }

    void incPurpleCoin() {
        bool isReached = GameSequenceFunction::getPlayResultInStageHolder()->incPurpleCoinNum();
        if (isReached) {
            EventFunction::stopCometEvent();
        }

        if (!isStagePurpleCoinsMoreThan100(getCurrentStageName())) {
            if (isReached) {
                requestAppearPowerStarCoin100();
            }
        }
    }

    s32 getPurpleCoinNum() {
        return GameSequenceFunction::getPlayResultInStageHolder()->getPurpleCoinNum();
    }

    bool isPurpleCoinNum100() {
        return GameSequenceFunction::getPlayResultInStageHolder()->getPurpleCoinNum() >= 100;
    }






    void onGameEventFlagBeeMarioAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("ハチマリオ初変身");
        }
    }

    void onGameEventFlagTeresaMarioAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("テレサマリオ初変身");
        }
    }

    void onGameEventFlagHopperMarioAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("ホッパーマリオ初変身");
        }
    }

    void onGameEventFlagFireMarioAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("ファイアマリオ初変身");
        }
    }

    void onGameEventFlagIceMarioAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("アイスマリオ初変身");
        }
    }

    void onGameEventFlagFlyingMarioAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("フライングマリオ初変身");
        }
    }

    void onGameEventFlagInvincibleMarioAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("無敵マリオ初変身");
        }
    }

    void onGameEventFlagLifeUpAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("ライフアップキノコ解説");
        }
    }

    void onGameEventFlagOneUpAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("１ＵＰキノコ解説");
        }
    }






    bool isOnGameEventFlagBeeMarioAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("ハチマリオ初変身");
    }

    bool isOnGameEventFlagTeresaMarioAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("テレサマリオ初変身");
    }

    bool isOnGameEventFlagHopperMarioAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("ホッパーマリオ初変身");
    }

    bool isOnGameEventFlagFireMarioAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("ファイアマリオ初変身");
    }

    bool isOnGameEventFlagIceMarioAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("アイスマリオ初変身");
    }

    bool isOnGameEventFlagFlyingMarioAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("フライングマリオ初変身");
    }

    bool isOnGameEventFlagInvincibleMarioAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("無敵マリオ初変身");
    }


    bool isOnGameEventFlagRockMarioAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("ゴロ岩マリオ初変身");
    }

    bool isOnGameEventFlagCloudMarioAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("雲マリオ初変身");
    }

    bool isOnGameEventFlagDrillAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("ドリル初ゲット");
    }

    bool isOnGameEventFlagLifeUpAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("ライフアップキノコ解説");
    }

    bool isOnGameEventFlagOneUpAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("１ＵＰキノコ解説");
    }

    bool isOnGameEventFlagTamakoroTutorialAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("タマコロチュートリアル");
    }

    bool isOnGameEventFlagGliBirdTutorialAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("グライバードチュートリアル");
    }

    bool isOnGameEventFlagYoshiFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("ヨッシー出会い");
    }

    bool isOnGameEventFlagCometMedalAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("コメットメダル解説");
    }

    bool isOnGameEventFlagSupportTicoAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("２Ｐサポート解説");
    }

    bool isOnGameEventFlagKinopioBankAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("銀行屋キノピオ初回");
    }

























    inline bool isOnGameEventFlagGalaxyOpen(const char* pKey) {
        return GameDataFunction::isOnGameEventFlag(pKey);
    }






    inline void onGameEventFlagGalaxyOpen(const char* pKey) {
        GameDataFunction::onGameEventFlag(pKey);
    }




    bool setupAlreadyDoneFlag(int* pOutIndex, const char* pName, const JMapInfoIter& rIter, s32 useless) {
        return GameSequenceFunction::getGameSequenceNameEntryTable()->setupAlreadyDoneEntry(pOutIndex, pName, rIter);
    }

    void onAlreadyDoneFlag(const char* pName) {
        GameSequenceFunction::getGameSequenceNameEntryTable()->onAlreadyDoneEntry(pName);
    }

    void updateAlreadyDoneFlag(int index) {
        GameSequenceFunction::getGameSequenceNameEntryTable()->updateAlreadyDoneEntry(index);
    }

    bool isOnAlreadyDoneFlag(const char* pName) {
        return GameSequenceFunction::getGameSequenceNameEntryTable()->isOnAlreadyDoneEntry(pName);
    }

    bool isAlreadyVisitedCurrentStageAndScenario() {
        return GameDataFunction::isOnGalaxyScenarioFlagAlreadyVisited(MR::getCurrentStageName(), MR::getCurrentScenarioNo());
    }


    bool isPowerStarGreenInCurrentStage(s32 starId) {
        GalaxyStatusAccessor accessor = makeGalaxyStatusAccessor(getCurrentStageName());
        return accessor.isGreenStar(starId);

    }


    bool isGrandStarInCurrentStage(s32 starId) {
        return GameDataConst::isGrandStar(MR::getCurrentStageName(), starId);
    }

    bool hasPowerStarInCurrentStageWithDeclarer(const char* pParam1, s32 param2) {
        return MR::isSuccessEventPowerStar(pParam1, param2);
    }

    bool isPowerStarGreenInCurrentStageWithDeclarer(const char* pParam1, s32 param2) {
        return MR::isGreenEventPowerStar(pParam1, param2);
    }


    bool isGrandStarInCurrentStageWithDeclarer(const char* pParam1, s32 param2) {
        return MR::isGrandEventPowerStar(pParam1, param2);
    }









    bool isOnGameEventFlagPowerStarSuccess(const char* pGalaxyName, s32 starId) {
        return GameDataFunction::hasPowerStar(pGalaxyName, starId);
    }

    bool hasGrandStar(int grandStarId) {
        return GameDataFunction::hasGrandStar(grandStarId);
    }

    bool isPowerStarGreen(const char* pGalaxyName, s32 starId) {
        GalaxyStatusAccessor accessor = makeGalaxyStatusAccessor(pGalaxyName);
        return accessor.isGreenStar(starId);

    }






    void setRaceBestTime(int id, u32 bestTime) {
        const char* pRaceName = RaceManagerFunction::getRaceName(id);

        GameDataFunction::setRaceBestTime(pRaceName, bestTime);
    }

    u32 getRaceBestTime(int id) {
        const char* pRaceName = RaceManagerFunction::getRaceName(id);

        return GameDataFunction::getRaceBestTime(pRaceName);
    }

    u32 getRaceCurrentTime() {
        return RaceManagerFunction::getRaceTime();
    }












    void updateStorageSpinDriverPathDrawRange(int param1, f32 param2) {
        const char* pStageName = MR::getCurrentStageName();
        s32 scenarioNo = MR::getCurrentScenarioNo();

        GameDataFunction::updateStorageSpinDriverPathDrawRange(pStageName, scenarioNo, param1, param2);
    }

    void startGalaxyCometEvent() {
        EventFunction::startCometEvent();
    }

    void endGalaxyCometEvent() {
        EventFunction::endCometEvent();
    }









    inline int getCometNameIdFromString(const char* pCometName) {
        if (MR::isEqualString(pCometName, "Red")) {
            return 0;
        }

        if (MR::isEqualString(pCometName, "Dark")) {
            return 2;
        }

        if (MR::isEqualString(pCometName, "Ghost")) {
            return 1;
        }

        if (MR::isEqualString(pCometName, "Quick")) {
            return 3;
        }

        if (MR::isEqualString(pCometName, "Purple")) {
            return 4;
        }

        if (MR::isEqualString(pCometName, "Black")) {
            return 4;
        }

        return 0;
    }





    inline s32 getCoinBestScore(const char* pGalaxyName) {
        GalaxyStatusAccessor accessor = MR::makeGalaxyStatusAccessor(pGalaxyName);
        s32 coinBestScore = 0;

        for (s32 i = 1; i <= accessor.getPowerStarNum(); i++) {
            if (!MR::isPlacedCoin(pGalaxyName, i)) {
                continue;
            }

            s32 coinBestScoreScenario = MR::getCoinBestScore(pGalaxyName, i);

            if (coinBestScore < coinBestScoreScenario) {
                coinBestScore = coinBestScoreScenario;
            }
        }

        return coinBestScore;
    }




















    void explainBeeMarioIfAtFirst() {
        if (!isOnGameEventFlagBeeMarioAtFirst()) {
            InformationObserverFunction::explainBee();
        }
    }

    void explainTeresaMarioIfAtFirst() {
        if (!isOnGameEventFlagTeresaMarioAtFirst()) {
            InformationObserverFunction::explainTeresa();
        }
    }

    void explainHopperMarioIfAtFirst() {
        if (!isOnGameEventFlagHopperMarioAtFirst()) {
            InformationObserverFunction::explainHopper();
        }
    }

    void explainFireMarioIfAtFirst() {
        if (!isOnGameEventFlagFireMarioAtFirst()) {
            InformationObserverFunction::explainFire();
        }
    }

    void explainIceMarioIfAtFirst() {
        if (!isOnGameEventFlagIceMarioAtFirst()) {
            InformationObserverFunction::explainIce();
        }
    }

    void explainFlyingMarioIfAtFirst() {
        if (!isOnGameEventFlagFlyingMarioAtFirst()) {
            InformationObserverFunction::explainFlying();
        }
    }

    void explainRockMarioIfAtFirst() {
        if (!isOnGameEventFlagRockMarioAtFirst()) {
            InformationObserverFunction::explainRock();
        }
    }

    void explainCloudMarioIfAtFirst() {
        if (!isOnGameEventFlagCloudMarioAtFirst()) {
            InformationObserverFunction::explainCloud();
        }
    }

    void explainDrillIfAtFirst() {
        if (!isOnGameEventFlagDrillAtFirst()) {
            InformationObserverFunction::explainDrill();
        }
    }

    void explainInvincibleMarioIfAtFirst() {
        if (!isOnGameEventFlagInvincibleMarioAtFirst()) {
            InformationObserverFunction::explainInvincible();
        }
    }

    void explainLifeUpIfAtFirst() {
        if (!isOnGameEventFlagLifeUpAtFirst()) {
            InformationObserverFunction::explainLifeUp();
        }
    }

    void explainOneUpIfAtFirst() {
        if (!isOnGameEventFlagOneUpAtFirst()) {
            InformationObserverFunction::explainOneUp();
        }
    }

    bool explainYoshiIfAtFirst(LiveActor* pHost) {
        if (isOnGameEventFlagYoshiFirst()) {
            return false;
        }

        InformationObserverFunction::explainYoshi(pHost);
        return true;
    }

    bool explainCometMedalIfAtFirst(LiveActor* pHost) {
        if (isOnGameEventFlagCometMedalAtFirst()) {
            return false;
        }

        InformationObserverFunction::explainCometMedal(pHost);
        return true;
    }









    void onGameEventFlagRockMarioAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("ゴロ岩マリオ初変身");
        }
    }

    void onGameEventFlagCloudMarioAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("雲マリオ初変身");
        }
    }

    void onGameEventFlagDrillAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("ドリル初ゲット");
        }
    }

    void onGameEventFlagTamakoroTutorialAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("タマコロチュートリアル");
        }
    }

    void onGameEventFlagGliBirdTutorialAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("グライバードチュートリアル");
        }
    }

    void onGameEventFlagYoshiAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("ヨッシー出会い");
        }
    }

    void onGameEventFlagCometMedalAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("コメットメダル解説");
        }
    }

    void onGameEventFlagSupportTicoAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("２Ｐサポート解説");
        }
    }

    void onGameEventFlagKinopioBankAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("銀行屋キノピオ初回");
        }
    }
















    inline bool isGalaxyRedCometAppearInCurrentStage() {
        return EventFunction::isStartCometEvent("Red");
    }

    bool isGalaxyDarkCometAppearInCurrentStage() {
        return EventFunction::isStartCometEvent("Dark");
    }

    inline bool isGalaxyGhostCometAppearInCurrentStage() {
        return EventFunction::isStartCometEvent("Ghost");
    }

    bool isGalaxyQuickCometAppearInCurrentStage() {
        return EventFunction::isStartCometEvent("Quick");
    }






    bool isPowerStarBronzeInCurrentStage(s32 starId) {
        GalaxyStatusAccessor accessor = makeGalaxyStatusAccessor(getCurrentStageName());
        return accessor.isOwnBronzeStar(starId);
    }

    bool isPowerStarBronze(const char* pStageName, s32 starId) {
        GalaxyStatusAccessor accessor = makeGalaxyStatusAccessor(pStageName);
        return accessor.isOwnBronzeStar(starId);
    }

    s32 getPowerStarNum() {
        return GameDataFunction::calcCurrentPowerStarNum();
    }

    bool hasAllNormalPowerStarWithoutComet(const char* pGalaxyName) {
        GalaxyStatusAccessor accessor = makeGalaxyStatusAccessor(pGalaxyName);
        for (s32 i = 1; i <= accessor.getNormalPowerStarNumWithoutComet(); i++) {
            if (!GameDataFunction::hasPowerStar(pGalaxyName, i)) {
                return false;
            }
        }

        return true;
    }

    bool hasPowerStarInCurrentStage(s32 starId) {
        return makeCurrentGalaxyStatusAccessor().hasPowerStar(starId);
    }

    bool isScenarioOpenInCurrentStageWithDeclarer(const char* pDeclarer) {
        s32 powerStarId = getEventPowerStarID(pDeclarer);
        return makeCurrentGalaxyStatusAccessor().isOpenScenario(powerStarId);
    }

    bool isPowerStarWithDeclarer(const char* pDeclarer, s32 powerStarId) {
        s32 eventPowerStarId = getEventPowerStarID(pDeclarer);
        return eventPowerStarId > 0 && eventPowerStarId == powerStarId;
    }

    bool isPowerStarBronzeInCurrentStageWithDeclarer(const char* pDeclarer, s32 powerStarId) {
        return isBronzeEventPowerStar(pDeclarer, powerStarId);
    }

    s32 getPowerStarNumOwnedInStage(const char* pStageName) {
        GalaxyStatusAccessor accessor = makeGalaxyStatusAccessor(pStageName);
        return accessor.getPowerStarNumOwnedTotal();
    }

    bool isPowerStarGreenInCurrentStage() {
        if (isProductGameScene()) {
            s32 scenarioNo = getCurrentOrSelectedScenarioNo();
            return isPowerStarGreen(getCurrentStageName(), scenarioNo);
        }

        return false;
    }

    bool isGalaxyCompletedNoGreen(const char* pStageName) {
        GalaxyStatusAccessor accessor = makeGalaxyStatusAccessor(pStageName);
        return accessor.isCompletedNoGreen();
    }

    bool isGalaxyCompletedWithGreen(const char* pStageName) {
        GalaxyStatusAccessor accessor = makeGalaxyStatusAccessor(pStageName);
        return accessor.isCompletedWithGreen();
    }

    const JMapIdInfo* getRestartMarioNo() {
        return GameSequenceFunction::getPlayResultInStageHolder()->mRestartMarioNo;
    }

    void setRestartMarioNo(const JMapIdInfo& rIdInfo) {
        GameSequenceFunction::getPlayResultInStageHolder()->setRestartMarioNo(rIdInfo);
    }

    bool isGhostLuigiOpenInCurrentStage() {
        s32 scenarioNo = getCurrentOrSelectedScenarioNo();
        return GameSequenceFunction::checkAppearGhostLuigi(getCurrentStageName(), scenarioNo);
    }

    bool isPostmanTaskInProgressAtStage(const char* pStageName, s32 taskId) {
        if (isEqualString(pStageName, PostmanEvent::getTaskStage(taskId))) {
            return PostmanEvent::isTaskInProgress(taskId);
        }

        return false;
    }

    bool isPostmanTaskInProgressAtStageGroup6To7(const char* pStageName) {
        if (isPostmanTaskInProgressAtStage(pStageName, 6)) {
            return true;
        }

        if (isPostmanTaskInProgressAtStage(pStageName, 7)) {
            return true;
        }

        return false;
    }

    bool isPostmanTaskInProgressAtStageGroup8To9(const char* pStageName) {
        if (isPostmanTaskInProgressAtStage(pStageName, 8)) {
            return true;
        }

        if (isPostmanTaskInProgressAtStage(pStageName, 9)) {
            return true;
        }

        return false;
    }

    bool isPostmanTaskInProgressAtStageGroup10To12(const char* pStageName) {
        if (isPostmanTaskInProgressAtStage(pStageName, 10)) {
            return true;
        }

        if (isPostmanTaskInProgressAtStage(pStageName, 11)) {
            return true;
        }

        if (isPostmanTaskInProgressAtStage(pStageName, 12)) {
            return true;
        }

        return false;
    }

    bool isIslandFleetGalaxy1FirstTime() {
        if (!isEqualStageName("IslandFleetGalaxy")) {
            return false;
        }

        if (getCurrentScenarioNo() != 1) {
            return false;
        }

        return GameDataFunction::calcCurrentPowerStarNum() == 0;
    }

    bool isGalaxyPurpleCometAppearInCurrentStage() {
        return EventFunction::isStartCometEvent("Purple");
    }

    bool isGalaxyHorrorCometAppearInCurrentStage() {
        return false;
    }

    bool isGalaxyRainbowCometAppearInCurrentStage() {
        return false;
    }

    bool isGalaxyCometEvent() {
        return EventFunction::isCometEvent();
    }

    void stopGalaxyCometEvent() {
        EventFunction::stopCometEvent();
    }

    bool getGalaxyStarTotalNum(const char* pGalaxyName) {
        GalaxyStatusAccessor accessor = makeGalaxyStatusAccessor(pGalaxyName);
        if (accessor.getPowerStarNum() == 0) {
            return 0;
        }

        return GameDataFunction::isOnGalaxyFlagComet(pGalaxyName);
    }

    bool isAnyGalaxyAvailableWithStars() {
        if (GameDataFunction::isOnGameEventFlagCometExplain()) {
            return false;
        }

        ScenarioDataIter iter = makeBeginScenarioDataIter();
        while (!iter.isEnd()) {
            GalaxyStatusAccessor accessor = iter.makeAccessor();
            if (getGalaxyStarTotalNum(accessor.getName())) {
                return true;
            }

            iter.goNext();
        }

        return false;
    }

    bool isPowerStarNumWorld3Enough() {
        if (GameDataFunction::isOnLuigiIntrusivelyStart()) {
            return false;
        }

        if (GameSequenceFunction::isClearedGrandStarVsKoopaLv3GalaxyAtResult()) {
            return false;
        }

        return GameDataFunction::calcNormalPowerStarNumFromWorld(3) >= 3;
    }

    bool isPendingNormalEndingDemo() {
        bool isPending = false;
        if (GameDataFunction::isOnGameEventFlag("ノーマルエンディング実行")) {
            if (!GameDataFunction::isOnGameEventFlag("ノーマルエンディング後デモ")) {
                isPending = true;
            }
        }

        return isPending;
    }

    bool isPending120StarEndingDemo() {
        bool isPending = false;
        if (GameDataFunction::isOnGameEventFlag("スター120個エンディング実行")) {
            if (!GameDataFunction::isOnGameEventFlag("スター120個エンディング後デモ")) {
                isPending = true;
            }
        }

        return isPending;
    }

    s32 registerStorageSpinDriverPathDrawRange(const NameObj* pParam1, const JMapInfoIter& rIter, int param3, f32* pParam4) {
        return 0;
    }

    bool explainMarioFacePlanetAtFirst(LiveActor* pHost) {
        InformationObserverFunction::explainMarioFacePlanet(pHost);
        return true;
    }

};  // namespace MR
