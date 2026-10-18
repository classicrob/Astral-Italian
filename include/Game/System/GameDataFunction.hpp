#pragma once

#include <revolution/os.h>

class GameDataHolder;
class GameDataSomeScenarioAccessor;
class JMapInfoIter;
class GameDataPlayerStatus;
class GameEventValueChecker;
class SaveDataHandleSequence;

namespace GameDataFunction {
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
};  // namespace GameDataFunction
