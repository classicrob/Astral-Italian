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















    bool isPreventDisplayInformationMessage() {
        if (isStageStaffRoll()) {
            return true;
        }

        return GameSequenceFunction::isPadMode2();
    }

    s32 getPlayerLeft() {
        return GameDataFunction::getPlayerLeft();
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


    bool isOnGameEventFlagTamakoroTutorialAtFirst() {
        if (isPreventDisplayInformationMessage()) {
            return true;
        }

        return GameDataFunction::isOnGameEventFlag("タマコロチュートリアル");
    }

























    inline bool isOnGameEventFlagGalaxyOpen(const char* pKey) {
        return GameDataFunction::isOnGameEventFlag(pKey);
    }






    inline void onGameEventFlagGalaxyOpen(const char* pKey) {
        GameDataFunction::onGameEventFlag(pKey);
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









    inline s32 getPowerStarNumToOpenGalaxy(const char* pGalaxyName) {
        return GameDataConst::getPowerStarNumToOpenGalaxy(pGalaxyName);
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

    void explainInvincibleMarioIfAtFirst() {
        if (!GameDataFunction::isOnGameEventFlag("無敵マリオ初変身")) {
            InformationObserverFunction::explainInvincible();
        }
    }

    void explainLifeUpIfAtFirst() {
        if (!GameDataFunction::isOnGameEventFlag("ライフアップキノコ解説")) {
            InformationObserverFunction::explainLifeUp();
        }
    }

    void explainOneUpIfAtFirst() {
        if (!GameDataFunction::isOnGameEventFlag("１ＵＰキノコ解説")) {
            InformationObserverFunction::explainOneUp();
        }
    }









    void onGameEventFlagTamakoroTutorialAtFirst() {
        if (!isPreventDisplayInformationMessage()) {
            GameDataFunction::onGameEventFlag("タマコロチュートリアル");
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

    s32 registerStorageSpinDriverPathDrawRange(const NameObj* pParam1, const JMapInfoIter& rIter, int param3, f32* pParam4) {
        return 0;
    }

    bool explainMarioFacePlanetAtFirst(LiveActor* pHost) {
        InformationObserverFunction::explainMarioFacePlanet(pHost);
        return true;
    }

};  // namespace MR
