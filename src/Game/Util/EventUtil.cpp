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

void EventUtil_FORCE_MATCH_STRINGS() {
    MR::isEqualString("ハチマリオ初変身", "ハチマリオ初変身");
    MR::isEqualString("テレサマリオ初変身", "テレサマリオ初変身");
    MR::isEqualString("ホッパーマリオ初変身", "ホッパーマリオ初変身");
    MR::isEqualString("ファイアマリオ初変身", "ファイアマリオ初変身");
    MR::isEqualString("アイスマリオ初変身", "アイスマリオ初変身");
    MR::isEqualString("フライングマリオ初変身", "フライングマリオ初変身");
    MR::isEqualString("無敵マリオ初変身", "無敵マリオ初変身");
    MR::isEqualString("ライフアップキノコ解説", "ライフアップキノコ解説");
    MR::isEqualString("１ＵＰキノコ解説", "１ＵＰキノコ解説");
    MR::isEqualString("クッパ襲来後", "クッパ襲来後");
    MR::isEqualString("ピーチ城浮上後", "ピーチ城浮上後");
    MR::isEqualString("チコガイドデモ終了", "チコガイドデモ終了");
    MR::isEqualString("スピン権利", "スピン権利");
    MR::isEqualString("天球儀レクチャー", "天球儀レクチャー");
    MR::isEqualString("ギャラクシー移動レクチャー", "ギャラクシー移動レクチャー");
    MR::isEqualString("スターピースレクチャー", "スターピースレクチャー");
    MR::isEqualString("サーフィンチュートリアル", "サーフィンチュートリアル");
    MR::isEqualString("タマコロチュートリアル", "タマコロチュートリアル");
    MR::isEqualString("RosettaTalkAboutTico", "RosettaTalkAboutTico");
    MR::isEqualString("SpecialStarGrand7", "SpecialStarGrand7");
    MR::isEqualString("ViewCompleteEnding", "ViewCompleteEnding");
    MR::isEqualString("バトラー情報Ａ", "バトラー情報Ａ");
    MR::isEqualString("SpecialStarGreenAll", "SpecialStarGreenAll");
    MR::isEqualString("SpecialStarRed1", "SpecialStarRed1");
    MR::isEqualString("ViewNormalEnding", "ViewNormalEnding");
    MR::isEqualString("KoopaBattleVs3Galaxy", "KoopaBattleVs3Galaxy");
    MR::isEqualString("EventCometStarter", "EventCometStarter");
    MR::isEqualString("SpecialStarFindingLuigi3", "SpecialStarFindingLuigi3");
    MR::isEqualString("SpecialStarLuigiRescued", "SpecialStarLuigiRescued");
    MR::isEqualString("EventKinopioExplorerOrganize", "EventKinopioExplorerOrganize");
    MR::isEqualString("EventKinopioExplorerRescued", "EventKinopioExplorerRescued");
    MR::isEqualString("SpecialStarGrand5", "SpecialStarGrand5");
    MR::isEqualString("WarpPodSaveBits", "WarpPodSaveBits");
    MR::isEqualString("TicoGalaxyAlreadyTalk", "TicoGalaxyAlreadyTalk");
    MR::isEqualString("AstroDome", "AstroDome");
    MR::isEqualString("SpecialStarGrand%1d", "SpecialStarGrand%1d");
    MR::isEqualString("Dark", "Dark");
    MR::isEqualString("Ghost", "Ghost");
    MR::isEqualString("Quick", "Quick");
    MR::isEqualString("Purple", "Purple");
    MR::isEqualString("Black", "Black");
    MR::isEqualString("PowerStarComplete", "PowerStarComplete");
    MR::isEqualString("StarPieceCounterStop", "StarPieceCounterStop");
    MR::isEqualString("LuigiTalkAfterRescued", "LuigiTalkAfterRescued");
    MR::isEqualString("EggStarGalaxy", "EggStarGalaxy");
    MR::isEqualString("MessageAlreadyRead", "MessageAlreadyRead");
    MR::isEqualString("MsgLedPattern", "MsgLedPattern");
}

namespace GameDataFunction {
    bool isOnGameEventFlag(const char*);
    void onGameEventFlag(const char*);
};  // namespace GameDataFunction

namespace {
};  // namespace

namespace MR {















    void onGameEventFlagBeeMarioAtFirst() {
        GameDataFunction::onGameEventFlag("ハチマリオ初変身");
    }

    void onGameEventFlagTeresaMarioAtFirst() {
        GameDataFunction::onGameEventFlag("テレサマリオ初変身");
    }

    void onGameEventFlagHopperMarioAtFirst() {
        GameDataFunction::onGameEventFlag("ホッパーマリオ初変身");
    }

    void onGameEventFlagFireMarioAtFirst() {
        GameDataFunction::onGameEventFlag("ファイアマリオ初変身");
    }

    void onGameEventFlagIceMarioAtFirst() {
        GameDataFunction::onGameEventFlag("アイスマリオ初変身");
    }

    void onGameEventFlagFlyingMarioAtFirst() {
        GameDataFunction::onGameEventFlag("フライングマリオ初変身");
    }

    void onGameEventFlagInvincibleMarioAtFirst() {
        GameDataFunction::onGameEventFlag("無敵マリオ初変身");
    }

    void onGameEventFlagLifeUpAtFirst() {
        GameDataFunction::onGameEventFlag("ライフアップキノコ解説");
    }

    void onGameEventFlagOneUpAtFirst() {
        GameDataFunction::onGameEventFlag("１ＵＰキノコ解説");
    }






    bool isOnGameEventFlagBeeMarioAtFirst() {
        return GameDataFunction::isOnGameEventFlag("ハチマリオ初変身");
    }

    bool isOnGameEventFlagTeresaMarioAtFirst() {
        return GameDataFunction::isOnGameEventFlag("テレサマリオ初変身");
    }

    bool isOnGameEventFlagHopperMarioAtFirst() {
        return GameDataFunction::isOnGameEventFlag("ホッパーマリオ初変身");
    }

    bool isOnGameEventFlagFireMarioAtFirst() {
        return GameDataFunction::isOnGameEventFlag("ファイアマリオ初変身");
    }

    bool isOnGameEventFlagIceMarioAtFirst() {
        return GameDataFunction::isOnGameEventFlag("アイスマリオ初変身");
    }

    bool isOnGameEventFlagFlyingMarioAtFirst() {
        return GameDataFunction::isOnGameEventFlag("フライングマリオ初変身");
    }


    bool isOnGameEventFlagTamakoroTutorialAtFirst() {
        return GameDataFunction::isOnGameEventFlag("タマコロチュートリアル");
    }

























    inline bool isOnGameEventFlagGalaxyOpen(const char* pKey) {
        return GameDataFunction::isOnGameEventFlag(pKey);
    }






    inline void onGameEventFlagGalaxyOpen(const char* pKey) {
        GameDataFunction::onGameEventFlag(pKey);
    }


    bool isPowerStarGreenInCurrentStage(s32 starId) {
        return GameDataConst::isPowerStarGreen(MR::getCurrentStageName(), starId);
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


    bool isPowerStarGreen(const char* pGalaxyName, s32 starId) {
        return GameDataConst::isPowerStarGreen(pGalaxyName, starId);
    }






    u32 getRaceCurrentTime() {
        return RaceManagerFunction::getRaceTime();
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
        if (!GameDataFunction::isOnGameEventFlag("ハチマリオ初変身")) {
            InformationObserverFunction::explainBee();
        }
    }

    void explainTeresaMarioIfAtFirst() {
        if (!GameDataFunction::isOnGameEventFlag("テレサマリオ初変身")) {
            InformationObserverFunction::explainTeresa();
        }
    }

    void explainHopperMarioIfAtFirst() {
        if (!GameDataFunction::isOnGameEventFlag("ホッパーマリオ初変身")) {
            InformationObserverFunction::explainHopper();
        }
    }

    void explainFireMarioIfAtFirst() {
        if (!GameDataFunction::isOnGameEventFlag("ファイアマリオ初変身")) {
            InformationObserverFunction::explainFire();
        }
    }

    void explainIceMarioIfAtFirst() {
        if (!GameDataFunction::isOnGameEventFlag("アイスマリオ初変身")) {
            InformationObserverFunction::explainIce();
        }
    }

    void explainFlyingMarioIfAtFirst() {
        if (!GameDataFunction::isOnGameEventFlag("フライングマリオ初変身")) {
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
        GameDataFunction::onGameEventFlag("タマコロチュートリアル");
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






};  // namespace MR
