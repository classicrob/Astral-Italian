#pragma once

#include <revolution/os.h>

class GameDataHolder;
class GameDataSomeScenarioAccessor;
class JMapInfoIter;
class GameDataPlayerStatus;
class GameEventValueChecker;
class SaveDataHandleSequence;

namespace GameDataFunction {
    bool isOnLuigiIntrusivelyStart();
    void onLuigiIntrusivelyAppear();
    s32 getPlayerStatusCounter14();
    s32 getPendingSharedPlayerLeft();
    s32 getPlayerMissNum();
    u32 getBestScoreAttackCurrentStage(const char* stageName = nullptr);
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
    SaveDataHandleSequence* getSaveDataHandleSequence();
    GameDataPlayerStatus* getCurrentGameDataPlayerStatus();
    GameEventValueChecker* getCurrentGameEventValueChecker();
    bool hasPowerStar(const char*, s32);
    bool hasGrandStar(int);
    bool isOnGalaxyScenarioFlagAlreadyVisited(const char*, s32);
    void onGalaxyScenarioFlagAlreadyVisited(const char*, s32);
    s32 calcCurrentPowerStarNum();
    s32 getPowerStarNumOwned(const char*);
    const char* getWorldFinalGalaxyName(s32 worldId);
    bool isGalaxyStateNew(const char*);
    bool isGalaxyStateOpened(const char*);
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
    bool isOnGameEventFlagFinalGalaxyAppearance();
    bool isOnGameEventFlagCoinCounterStop();
    bool isOnGameEventFlagCometExplain();
    s32 calcCurrentGrandStarNum();
    void incCumulativePlayTime();
    void setCurrentWorldId(u8);
};  // namespace GameDataFunction
