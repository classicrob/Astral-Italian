#pragma once

#include <revolution/os.h>

class GameDataHolder;
class GameDataSomeScenarioAccessor;
class JMapInfoIter;
class GameDataPlayerStatus;
class GameEventValueChecker;
class SaveDataHandleSequence;

namespace GameDataFunction {
    void storeSharedPlayerLeft();
    void resetSysConfigCounters();
    bool hasBronzeStar(const char*, s32);
    bool existsGameEventFlagOpenScenario(const char*, s32);
    bool getSysConfigFileEncouragedPal16();
    void setSysConfigFileEncouragedPal16();
    u32 getScenarioClearTime(const char*, s32);
    u32 getBestScoreAttackCurrentStage(const char* pStageName = nullptr);  // SMG2 symbol FPCc (core h10)
    bool isOnLuigiIntrusivelyStart();
    void onLuigiIntrusivelyAppear();
    s32 getPlayerStatusCounter14();
    s32 getPendingSharedPlayerLeft();
    s32 getPlayerMissNum();
    void setBestScoreAttackCurrentStage(u32);
    s32 getTicoFatStarPieceAlms(s32);
    void addTicoFatStarPieceAlms(s32, s32);
    bool isOnMessageAlreadyRead(s8);
    void setOnMessageAlreadyRead(s8);
    s32 calcTicoCoinNum();
    void onTicoCoinFeed(const char*);
    bool isTicoCoinFeed(const char*);
    s32 getBankedStarPieceNum();
    s32 getMaxBankedStarPieceNum();
    s32 getBankInterest();
    void clearKinopioBankInterest();
    void setBankedStarPieceNum(s32);
    void tryOnGameEventFlagStarPieceCounterStop(int);
    bool isOnPostmanImportantLetterRead(s32);
    bool isOnPostmanTaskLetterRead(s32);
    bool isOnGalaxyFlagTicoCoin(const char* pStageName);
    bool isOnGameEventFlagStarPieceCounterStop();
}

// Merged from Petari (core lane, symbols.txt-verified)

namespace GameDataFunction {
    SaveDataHandleSequence* getSaveDataHandleSequence() NO_INLINE;
    GameDataPlayerStatus* getCurrentGameDataPlayerStatus();
    GameEventValueChecker* getCurrentGameEventValueChecker();
    bool hasPowerStar(const char*, s32);
    bool hasGrandStar(int);
    bool isOnGalaxyScenarioFlagAlreadyVisited(const char*, s32);
    void onGalaxyScenarioFlagAlreadyVisited(const char*, s32);
    s32 calcCurrentPowerStarNum();
    s32 getPowerStarNumOwned(const char*) NO_INLINE;
    const char* getWorldFinalGalaxyName(s32 worldId);
    s32 getGrandStarNo(const char* pGalaxyName);
    s32 getCometAppearMedalNum(const char* pGalaxyName);
    bool isGalaxyStateNew(const char*);
    bool isGalaxyStateOpened(const char*);
    bool existsGameEventFlagOpenScenario(const char*, s32);
    void setGalaxyStateNew(const char*);
    void setGalaxyStateOpened(const char*);
    s32 getPowerStarNumMax();
    void onGameEventFlag(const char*);
    bool isOnGameEventFlag(const char*);
    void setRaceBestTime(const char*, u32);
    s32 getRaceBestTime(const char*);
    void incPlayerMissNum();
    void updateStorageSpinDriverPathDrawRange(const char*, int, int, f32);
    s32 getPlayerLeft();
    void addPlayerLeft(int);
    s32 getStockedStarPieceNum();
    s32 getStockedCoinNum();
    const wchar_t* getUserName();
    void resetAllGameData();
    OSTime getSysConfigFileTimeSent();
    u32 getSysConfigFileSentBytes();
    void setSysConfigFileSentBytes(u32);
    s32 getCurrentWorldId();
};  // namespace GameDataFunction

// SMG2-only (core lane): declared for the GameDataFunction unit
namespace GameDataFunction {
    bool isOnGameEventFlagEnding120Stars();
    bool isOnGameEventFlagFinalGalaxyAppearance() NO_INLINE;
    bool isOnGameEventFlagCoinCounterStop();
    bool isOnGameEventFlagCometExplain();
    s32 calcCurrentGrandStarNum();
    void incCumulativePlayTime();
    void setCurrentWorldId(u8);
};  // namespace GameDataFunction

namespace GameDataFunction { s32 getFaceShipEventGrandStarNo(); u32 getStageClearsAfterRepeatedMissCount(); void setStageClearsAfterRepeatedMissCount(u16); void incStageClearsAfterRepeatedMissCount(); }

namespace GameDataFunction {
    void setOnPostmanImportantLetterRead(s32);
    void setOnPostmanTaskLetterRead(s32);
    void updatePostmanPeachLetterDeath();
    bool isOnGameEventFlagOpenScenario(const char*, s32);
    void onGameEventFlagOpenScenario(const char*, s32);
}
// GameSequenceFunction lane (core): declarations needed by GameSequenceFunction; names given from the target code.
namespace GameDataFunction {
    void forceMarioPlayer();
    void forceLuigiPlayer();
    bool isLuigiPlayer();
    s32 getScenarioMissNum(const char*, s32);
    bool isOnGameEventFlagWorld1Entered();
    void offGameEventFlag(const char*);
    s32 tryOnGameEventFlagCoinCounterStop(int);
    void setPowerStar(const char*, s32);
    void setBronzeStar(const char*, s32);
    s32 getNormalPowerStarNumOwned();
    bool isGalaxyStateClosed(const char*);
    bool isOnGalaxyFlagComet(const char*);
    void onGalaxyFlagComet(const char*);
    void offGalaxyFlagComet(const char*);
    void onGalaxyFlagTicoCoin(const char*);
    bool isOnGalaxyScenarioFlagGhostLuigi(const char*, s32);
    void onGalaxyScenarioFlagGhostLuigi(const char*, s32);
    void updateBankInterest();
    s32 calcNormalPowerStarNumFromWorld(s32);       // fn_804D27F0
    bool isOnLuigiIntrusivelyStart();    // fn_804D30E0
    bool isOnCurrentScenarioFlagLuigiIntruded();    // fn_804D3190
    void updateLuigiIntrusionCounter();             // fn_804D31E0
    void setScenarioBestTime(const char*, s32, s32);  // fn_804D3CB0
    u32 getScenarioClearTime(const char*, s32);      // fn_804D3D10
    void setFaceShipEventGrandStarNo(s32);          // fn_804D41D0
    void clearFaceShipEventValues();                // fn_804D4210
    bool isSysConfigCounterMax();                   // fn_804D46C0
    void copySysConfigData();                       // fn_804D4730
};  // namespace GameDataFunction

// core h10 (GameDataFunction unit): SMG2 declarations of functions defined in GameDataFunction.cpp
namespace GameDataFunction {
    void offGameEventFlag(const char*);
    void startLuigiIntrusion();
    void setOnPostmanTaskLetterRead(s32);
    void setOnPostmanImportantLetterRead(s32);
    void updatePostmanPeachLetterDeath();
    s32 getBankInterest();
    void clearKinopioBankInterest();
    s32 getPlayerMissNum();
    void incScenarioMissNum(const char*, s32);
    void incPlayerGameOverNum();
    s32 getPlayerGameOverNum();
    s32 getTicoFatStarPieceAlmsFromWorld(s32, s32);    // SMG2 fn_804D3980 (core h10)
    s32 getPlayerStatusCounter14();                    // SMG2 fn_804D3B20
    s32 getCumulativePlayTime();
    void resetPlayerLeftAndTryOnDimensionBigCastleGalaxyComet();  // SMG2 fn_804D3DF0 (provisional name)
    bool isOnWorldMapPointFlag(s32, s32);              // SMG2 fn_804D3E20 (provisional name)
    void onWorldMapPointFlag(s32, s32);                // SMG2 fn_804D3E70 (provisional name)
    void offWorldMapPointFlag(s32, s32);               // SMG2 fn_804D3EC0 (provisional name)
    s32 calcLastEnteredWorldNo();                      // SMG2 fn_804D3F80 (provisional name)
    s32 calcFaceShipEventNum();                        // SMG2 fn_804D4050 (provisional name)
    s32 getFaceShipEventNo(s32);                       // SMG2 fn_804D40C0 (provisional name)
    void tryAddFaceShipEventNo(s32);                   // SMG2 fn_804D4100 (provisional name)
    s32 getFaceShipEventGrandStarNo();                 // SMG2 fn_804D4180
    void setStageClearsAfterRepeatedMissCount(u16);    // SMG2 fn_804D42B0
    u32 getStageClearsAfterRepeatedMissCount();        // SMG2 fn_804D4280
    void incStageClearsAfterRepeatedMissCount();       // SMG2 fn_804D42F0
    void setUserName(const wchar_t*);
    bool hasCurrentUserMiiId();                        // SMG2 fn_804D43C0 (provisional name)
    void setSysConfigFileTimeSent(OSTime);
};  // namespace GameDataFunction
