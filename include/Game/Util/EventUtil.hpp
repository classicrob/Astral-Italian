#pragma once

#include <JSystem/JGeometry/TVec.hpp>
#include <revolution.h>

class JMapIdInfo;
class JMapInfoIter;
class LiveActor;
class NameObj;

namespace MR {
    bool isOnGameEventFlagYoshiFirst();
    bool isUseScenarioSelectMusicZeroStars();
    bool isUseScenarioSelectMusicOneStar();
    bool branchFuncGameProgress(u32);
    bool isPostmanTaskEventAvailable(s32);
    bool isPostmanTaskEventFinished(s32);
    bool isPostmanImportantEventAvailable(s32);
    s32 getPostmanImportantEventReward(s32);
    bool isValidPostmanImportantEvent(s32);
    s32 getScoreAttackScore();
    void setExterminationWoodBoxFlag();
    void clearExterminationWoodBoxFlag();
    bool isPostmanSpecialLetter(s32);
    void incPlayerAndTryScenarioMissNum(bool isGameOver);
    s32 getPlayerLeft();
    void addPlayerLeft(int lives);
    void decPlayerLeft();
    void incPlayerGameOverNum();

    bool isPlayerLuigi();

    s32 getStarPieceNum();
    void addStarPiece(int itemCount);
    void incCoin(int itemCount, LiveActor* pHost);
    void decCoin(int itemCount);
    s32 getCoinNum();
    void incPurpleCoin();
    s32 getPurpleCoinNum();
    bool isPurpleCoinNum100();

    void explainBeeMarioIfAtFirst();
    void explainTeresaMarioIfAtFirst();
    void explainHopperMarioIfAtFirst();
    void explainFireMarioIfAtFirst();
    void explainIceMarioIfAtFirst();
    void explainFlyingMarioIfAtFirst();
    void explainInvincibleMarioIfAtFirst();
    void explainRockMarioIfAtFirst();
    void explainCloudMarioIfAtFirst();
    void explainDrillIfAtFirst();
    void explainLifeUpIfAtFirst();
    void explainOneUpIfAtFirst();
    bool explainYoshiIfAtFirst(LiveActor* pHost);
    bool explainCometMedalIfAtFirst(LiveActor* pHost);
    bool explainMarioFacePlanetAtFirst(LiveActor* pHost);

    void onGameEventFlagBeeMarioAtFirst();
    void onGameEventFlagTeresaMarioAtFirst();
    void onGameEventFlagHopperMarioAtFirst();
    void onGameEventFlagFireMarioAtFirst();
    void onGameEventFlagIceMarioAtFirst();
    void onGameEventFlagFlyingMarioAtFirst();
    void onGameEventFlagInvincibleMarioAtFirst();
    void onGameEventFlagRockMarioAtFirst();
    void onGameEventFlagCloudMarioAtFirst();
    void onGameEventFlagDrillAtFirst();
    void onGameEventFlagLifeUpAtFirst();
    void onGameEventFlagOneUpAtFirst();
    void onGameEventFlagTamakoroTutorialAtFirst();
    void onGameEventFlagGliBirdTutorialAtFirst();
    void onGameEventFlagYoshiAtFirst();
    void onGameEventFlagCometMedalAtFirst();
    void onGameEventFlagSupportTicoAtFirst();
    void onGameEventFlagKinopioBankAtFirst();

    bool isOnGameEventFlagBeeMarioAtFirst();
    bool isOnGameEventFlagTeresaMarioAtFirst();
    bool isOnGameEventFlagHopperMarioAtFirst();
    bool isOnGameEventFlagFireMarioAtFirst();
    bool isOnGameEventFlagIceMarioAtFirst();
    bool isOnGameEventFlagFlyingMarioAtFirst();
    bool isOnGameEventFlagInvincibleMarioAtFirst();
    bool isOnGameEventFlagRockMarioAtFirst();
    bool isOnGameEventFlagCloudMarioAtFirst();
    bool isOnGameEventFlagDrillAtFirst();
    bool isOnGameEventFlagLifeUpAtFirst();
    bool isOnGameEventFlagOneUpAtFirst();
    bool isOnGameEventFlagTamakoroTutorialAtFirst();
    bool isOnGameEventFlagGliBirdTutorialAtFirst();
    bool isOnGameEventFlagYoshiFirst();
    bool isOnGameEventFlagCometMedalAtFirst();
    bool isOnGameEventFlagSupportTicoAtFirst();
    bool isOnGameEventFlagKinopioBankAtFirst();
    bool isOnGameEventFlagNormalEnding();
    bool isOnGameEventFlag120PowerStarsEnding();

    bool isPreventDisplayInformationMessage();

    bool setupAlreadyDoneFlag(int* outIndex, const char* name, const JMapInfoIter& rIter, s32 useless);
    void onAlreadyDoneFlag(const char* name);
    void onAlreadyDoneFlag(int index);
    bool isOnAlreadyDoneFlag(const char* name);

    bool isAlreadyVisitedCurrentStageAndScenario();

    bool hasPowerStarInCurrentStage(s32 powerStarId);
    bool isPowerStarGreenInCurrentStage(s32 powerStarId);
    bool isPowerStarBronzeInCurrentStage(s32 powerStarId);
    bool isGrandStarInCurrentStage(s32 powerStarId);
    bool hasPowerStarInCurrentStageWithDeclarer(const char* pDeclarer, s32 powerStarId);
    bool isPowerStarGreenInCurrentStageWithDeclarer(const char* pDeclarer, s32 powerStarId);
    bool isPowerStarBronzeInCurrentStageWithDeclarer(const char* pDeclarer, s32 powerStarId);
    bool isGrandStarInCurrentStageWithDeclarer(const char* pDeclarer, s32 powerStarId);
    bool isScenarioOpenInCurrentStageWithDeclarer(const char* pDeclarer);
    bool isPowerStarWithDeclarer(const char* pDeclarer, s32 powerStarId);
    bool isOnGameEventFlagPowerStarSuccess(const char* pDeclarer, s32 powerStarId);
    bool hasGrandStar(int grandStarId);
    s32 getPowerStarNumOwnedInStage(const char* pStageName);
    bool isPowerStarGreen(const char* pStageName, s32 powerStarId) NO_INLINE;
    bool isPowerStarGreenInCurrentStage();
    bool isPowerStarBronze(const char* pStageName, s32 powerStarId);

    s32 getPowerStarNum();
    bool hasAllNormalPowerStarWithoutComet(const char* pStageName);

    void setRaceBestTime(int raceId, s32 bestTime);
    unsigned long getRaceBestTime(int raceId);
    unsigned long getRaceCurrentTime();

    s32 registerStorageSpinDriverPathDrawRange(const NameObj*, const JMapInfoIter&, int, f32*);  // useless
    void updateStorageSpinDriverPathDrawRange(int, f32);                                         // useless

    bool isGalaxyDarkCometAppearInCurrentStage();
    bool isGalaxyQuickCometAppearInCurrentStage();
    bool isGalaxyPurpleCometAppearInCurrentStage();
    bool isGalaxyHorrorCometAppearInCurrentStage();   // always false
    bool isGalaxyRainbowCometAppearInCurrentStage();  // always false

    void startGalaxyCometEvent();
    void endGalaxyCometEvent();
    bool isGalaxyCometEvent();
    void stopGalaxyCometEvent();  // PLACEHOLDER(core): name inferred from symbol order (0x80024F50 between isGalaxyCometEvent and getGalaxyStarTotalNum); confirm

    // various unknown functions

    bool isGalaxyCompletedNoGreen(const char* pStageName);
    bool isGalaxyCompletedWithGreen(const char* pStageName);

    const JMapIdInfo* getRestartMarioNo();
    void setRestartMarioNo(const JMapIdInfo& rIdInfo);

    bool isGhostLuigiOpenInCurrentStage();

    // various PostmanEvent functions

    bool isIslandFleetGalaxy1FirstTime();
};  // namespace MR

// Merged from Petari (core lane): declarations missing here
#include <revolution/types.h>

namespace MR {
    void incPlayerLeft();
    bool isPlayerLeftSupply();
    bool isLuigiLeftSupply();
    bool isPlayerLeftSupplyByMissAndGameOver();
    bool isAnyPlayerLeftSupply();
    void offAllPlayerLeftSupply();
    s32 getStockedStarPieceNum();
    void addStockedStarPiece(int);
    void explainEnableToSpin(LiveActor*);
    bool isOnGameEventFlagPlayMoviePrologueA();
    bool isOnGameEventFlagEndTicoGuideDemo();
    bool isOnGameEventFlagEndButlerDomeLecture();
    bool isOnGameEventFlagEndButlerGalaxyMoveLecture();
    bool isOnGameEventFlagEndButlerStarPieceLecture();
    bool isOnGameEventFlagSurfingTutorialAtFirst();
    bool isOnGameEventFlagViewCompleteEnding();
    bool isOnGameEventFlagViewCompleteEndingMarioAndLuigi();
    bool isOnGameEventFlagOffAstroDomeGuidance();
    bool isOnGameEventFlagChildsRoom();
    bool isOnGameEventFlagLibraryRoom();
    bool isOnGameEventFlagRosettaTalkAboutTicoInTower();
    bool isRosettaTalkTorchProgress();
    bool isRosettaTalkCountDownStart();
    bool isRosettaTalkAstroDomeRecover();
    bool isRosettaTalkKoopa();
    bool isRosettaTalkTorchLecture();
    bool isKinopioExplorerCompleteTrickComet();
    bool isKinopioExplorerTalkGoFinalBattle();
    bool isKinopioExplorerTalkGetGrandStar6();
    bool isKinopioExplorerTalkGetGrandStar5();
    bool isKinopioExplorerTalkGetGrandStar4();
    bool isKinopioExplorerTalkGetGrandStar3();
    bool isKinopioExplorerTrickComet();
    bool isKinopioExplorerTalkGetGrandStar2();
    bool isUFOKinokoBeforeConstruction();
    bool isUFOKinokoUnderConstruction();
    bool isButlerMapAppear();
    void updateAlreadyDoneFlag(int, u32);
    void updateAlreadyDoneFlag(int);
    bool isOnGameEventFlagGalaxyOpen(const char*);
    bool isAlreadyVisitedStage(const char*);
    bool canOpenGalaxy(const char*);
    bool isAppearGalaxy(const char*);
    bool isGalaxyAppearGreenDriver(const char*);
    void onGameEventFlagGalaxyOpen(const char*);
    bool isPowerStarRedInCurrentStage(s32);
    bool isPowerStarRedInCurrentStageWithDeclarer(const char*, s32);
    bool hasPowerStarAtResultSequence();
    bool isPowerStarGreenAtResultSequence();
    bool isPowerStarRedAtResultSequence();
    void sendStageResultSequenceParam(s32);
    s32 calcOpenedAstroDomeNum();
    s32 calcCurrentGreenStarNum();
    s32 getPowerStarNumToOpenGalaxy(const char*);
    s32 getPowerStarNumSucceed(const char*);
    s32 getPictureBookChapterCanRead();
    s32 getPictureBookChapterAlreadyRead();
    void setPictureBookChapterAlreadyRead(int);
    void setWarpPodPathFlag(int, bool);
    s32 getStarPieceNumGivingToTicoSeed(int);
    void addStarPieceGivingToTicoSeed(int, int);
    s32 getStarPieceNumGivingToTicoGalaxy(int);
    s32 getStarPieceNumMaxGivingToTicoGalaxy(int);
    void addStarPieceGivingToTicoGalaxy(int, int);
    void setTicoGalaxyAlreadyTalk(int, bool);
    bool isGalaxyAnyCometAppearInCurrentStage();
    s32 getGalaxyCometStateIndexInCurrentStage();
    void forceToNextStateGalaxyCometScheduler();
    bool isGalaxyCometLandInStage(const char*);
    s32 getEncounterGalaxyCometPowerStarId(const char*);
    bool isGalaxyPurpleCometLaunch();
    bool canAppearNormalComet(const char*);
    bool canAppearCoin100Comet(const char*);
    int getEncounterGalaxyCometNameId(const char*);
    int getCometNameIdFromString(const char*);
    const char* getCometNameFromId(int);
    bool isStarComplete(const char*);
    bool isStarCompleteNormalScenario(const char*);
    s32 getCoinBestScore(const char*, s32);
    bool isPlacedCoin(const char*, s32);
    bool isActiveLuigiHideAndSeekEvent();
    bool isEndLuigiHideAndSeekEvent();
    bool isLuigiDisappearFromAstroGalaxy();
    bool isOnGameEventFlagLuigiRescued();
    void onGameEventFlagTalkedToLuigiAfterRescued();
    void onGameEventFlagGetLuigiLetter();
    bool isOnLuigiHiding();
    bool isOnLuigiHidingCurrentStage();
    bool isLuigiDisappearFromAstroGalaxyOrHiding();
    bool isLuigiLetterArrivalAtMessenger();
    bool isLuigiHidingGalaxyAndScenario(const char*, s32);
    bool isPowerStarGetDemoWithLuigiCurrentGalaxy();
    bool isPowerStarGetDemoWithLuigiCurrentGalaxyAndScenario(s32);
    const char* getLuigiLetterGalaxyName();
    const char* getLuigiLetterGalaxyNameForNPC();
    void onMessageAlreadyRead(s8);
    void offMsgLedPattern();
    bool isMsgLedPattern();
    void onGameEventFlagPlayMoviePrologueA();
    void onGameEventFlagPlayMoviePrologueB();
    void onGameEventFlagEndTicoGuideDemo();
    void onGameEventFlagEnableToSpinAndStarPointer();
    void onGameEventFlagEndButlerDomeLecture();
    void onGameEventFlagEndButlerGalaxyMoveLecture();
    void onGameEventFlagEndButlerStarPieceLecture();
    void onGameEventFlagSurfingTutorialAtFirst();
    void onGameEventFlagRosettaTalkAboutTicoInTower();
    void onGameEventFlagViewNormalEnding();
    void onGameEventFlagViewCompleteEnding();
    bool isOnGameEventFlagUseAstroDome();
    bool isOnGameEventFlagGreenDriver();
    bool isOnGameEventFlagRedDriver();
    bool isOnGameEventFlagViewNormalEnding();
    bool isRosettaTalkTrickComet();
    bool isKinopioExplorerStartMessenger();
    bool isKinopioExplorerOrganize();
    bool isKinopioExplorerRescued();
    s32 getPowerStarLeftToDisplayCountDownPlate();
    bool isOnWarpPodPathFlag(int);
    bool isOnTicoGalaxyAlreadyTalk(int);
    bool isKoopaFortressAppearInGalaxy();
    bool isGalaxyRedCometAppearInCurrentStage();
    bool isGalaxyGhostCometAppearInCurrentStage();
    bool isGalaxyBlackCometAppearInCurrentStage();
    bool isStarCompleteAllGalaxy();
    bool isStarPieceCounterStop();
    bool isOnGameEventFlagAstroGalaxyBgmBright();
    bool isOnMessageAlreadyRead(s8);
    void onMsgLedPattern();
};  // namespace MR

// Merged from Petari (core lane): overloads missing here


namespace MR {
    void setRaceBestTime(int, u32);
    s32 getGalaxyStarTotalNum(const char* pGalaxyName);
};  // namespace MR

namespace MR {
    bool hasMarioFaceShipItemEventAtResult();
}

namespace MR { bool isPendingNormalEndingDemo(); bool isPending120StarEndingDemo(); }

namespace MR {
    // PLACEHOLDER(core): unnamed fn_80024FB0 / fn_80025030 / fn_80025260 / fn_800252C0 / fn_80025320 / fn_80025380; names are guesses from callers, resolve when the owners are named
    bool isAnyGalaxyAvailableWithStars();
    bool isPowerStarNumWorld3Enough();
    bool isPostmanTaskInProgressAtStage(const char* pStageName, s32 taskId);
    bool isPostmanTaskInProgressAtStageGroup6To7(const char* pStageName);
    bool isPostmanTaskInProgressAtStageGroup8To9(const char* pStageName);
    bool isPostmanTaskInProgressAtStageGroup10To12(const char* pStageName);
};  // namespace MR

namespace MR {
    // SMG2 fn_80024FB0 (core4 name): comet-explain flag is off and some galaxy of the scenario data holds stars.
    bool isCometExplainPendingWithStarGalaxy();
    // SMG2 fn_80025030 (core4 name): not Luigi-intrusive start, Koopa Lv3 not cleared at result and 3 normal stars of World 3.
    bool isWorld3NormalStarsGatheredForFaceShip();
}

namespace MR {
    // PLACEHOLDER(m1): event predicate name unknown; resolve when MarioFaceShip event semantics are recovered.
}

namespace MR {
    void incYoshiFruitEaten();
}
