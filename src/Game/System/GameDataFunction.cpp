#include "Game/System/GameDataFunction.hpp"
#include "Game/Util.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameDataGalaxyStorage.hpp"
#include "Game/System/GameDataHolder.hpp"
#include "Game/System/GameDataTemporaryInGalaxy.hpp"
#include "Game/System/GameDataPlayerStatus.hpp"
#include "Game/System/GameEventFlagTable.hpp"
#include "Game/System/GameEventValueChecker.hpp"
#include "Game/System/GameSequenceDirector.hpp"
#include "Game/System/GameSequenceProgress.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/SaveDataHandleSequence.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/System/SysConfigFile.hpp"
#include "Game/System/UserFile.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <cstdio>

namespace {
    SaveDataHandleSequence* getSaveDataHandleSequence() NO_INLINE {
        return SingletonHolder< GameSystem >::get()->mSequenceDirector->mSaveDataHandleSequence;
    }

    GameDataHolder* getSceneStartGameDataHolder() {
        return ::getSaveDataHandleSequence()->getBackupUserFile()->mGameDataHolder;
    }

    GameDataTemporaryInGalaxy* getGameDataTemporaryInGalaxy() {
        return SingletonHolder< GameSystem >::get()->mSequenceDirector->mGameDataTemporaryInGalaxy;
    }

    GameSequenceProgress* getGameSequenceProgress() NO_INLINE {
        return SingletonHolder< GameSystem >::get()->mSequenceDirector->mGameSequenceProgress;
    }
};  // namespace

namespace GameDataFunction {
    SaveDataHandleSequence* getSaveDataHandleSequence() NO_INLINE {
        return SingletonHolder< GameSystem >::get()->mSequenceDirector->mSaveDataHandleSequence;
    }

    GameDataHolder* getCurrentGameDataHolder() {
        return ::getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder;
    }

#pragma dont_inline on
    GameDataPlayerStatus* getCurrentGameDataPlayerStatus() {
        return getCurrentGameDataHolder()->mPlayerStatus;
    }

    GameEventValueChecker* getCurrentGameEventValueChecker() {
        return getCurrentGameDataHolder()->mEventValueChecker;
    }
#pragma dont_inline reset

    bool hasPowerStar(const char* pGalaxyName, s32 scenarioNo) {
        return getCurrentGameDataHolder()->hasPowerStar(pGalaxyName, scenarioNo);
    }

    bool hasGrandStar(int id) {
        return getCurrentGameDataHolder()->hasGrandStar(id);
    }



    bool isOnGalaxyScenarioFlagAlreadyVisited(const char* pGalaxyName, s32 scenarioNo) {
        return getCurrentGameDataHolder()->isOnGalaxyScenarioFlagAlreadyVisited(pGalaxyName, scenarioNo);
    }

    void onGalaxyScenarioFlagAlreadyVisited(const char* pGalaxyName, s32 scenarioNo) {
        getCurrentGameDataHolder()->onGalaxyScenarioFlagAlreadyVisited(pGalaxyName, scenarioNo);
    }


    s32 calcCurrentPowerStarNum() {
        return getCurrentGameDataHolder()->calcCurrentPowerStarNum();
    }

    s32 getPowerStarNumOwned(const char* pGalaxyName) {
        return getCurrentGameDataHolder()->getPowerStarNumOwned(pGalaxyName);
    }

    s32 getPowerStarNumMax() {
        s32 powerStarNumMax = 0;

        for (ScenarioDataIter iter = MR::makeBeginScenarioDataIter(); !iter.isEnd(); iter.goNext()) {
            powerStarNumMax += iter.makeAccessor().getPowerStarNum();
        }

        return powerStarNumMax;
    }




    void onGameEventFlag(const char* pName) {
        getCurrentGameDataHolder()->tryOnGameEventFlag(pName);
    }



    bool isOnGameEventFlag(const char* pName) {
        return getCurrentGameDataHolder()->isOnGameEventFlag(pName);
    }








    void setRaceBestTime(const char* pRaceName, u32 bestTime) {
        getCurrentGameDataHolder()->setRaceBestTime(pRaceName, bestTime);
    }

    s32 getRaceBestTime(const char* pRaceName) {
        return getCurrentGameEventValueChecker()->getU32Value(pRaceName);
    }




    void incPlayerMissNum() {
        getCurrentGameDataHolder()->incPlayerMissNum();
    }


    void updateStorageSpinDriverPathDrawRange(const char* pGalaxyName, int scenarioNo, int param3, f32 param4) {
        getCurrentGameDataHolder()->updateSpinDriverPathStorage(pGalaxyName, scenarioNo, param3, param4);
    }






    s32 getPlayerLeft() {
        return getCurrentGameDataPlayerStatus()->getPlayerLeft();
    }

    void addPlayerLeft(int num) {
        getCurrentGameDataPlayerStatus()->addPlayerLeft(num);
    }



    s32 getStockedStarPieceNum() {
        return getCurrentGameDataHolder()->getStockedStarPieceNum();
    }





    const wchar_t* getUserName() {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mUserName;
    }


    void resetAllGameData() {
        getCurrentGameDataHolder()->resetAllData();
    }







    s32 calcTicoGalaxyNum(const GameDataHolder* pGameDataHolder) {
        s32 ticoGalaxyNum = 0;
        s32 exclamationGalaxyNum = GameEventFlagTable::calcExclamationGalaxyNum();

        for (s32 i = 0; i < exclamationGalaxyNum; i++) {
            const char* pExclamationGalaxyName = GameEventFlagTable::getExclamationGalaxyNameFromIndex(i);

            if (pGameDataHolder->isAppearGalaxy(pExclamationGalaxyName)) {
                ticoGalaxyNum++;
            }
        }

        return ticoGalaxyNum;
    }

    s32 calcGrandStarNum(const GameDataHolder* pGameDataHolder) {
        s32 grandStarNum = 0;

        for (int i = 1; i <= 7; i++) {
            if (pGameDataHolder->hasGrandStar(i)) {
                grandStarNum++;
            }
        }

        return grandStarNum;
    }







    OSTime getSysConfigFileTimeSent() {
        return getSaveDataHandleSequence()->getSysConfigFile()->getTimeSent();
    }

    void setSysConfigFileTimeSent(OSTime timeSent) {
        getSaveDataHandleSequence()->getSysConfigFile()->setTimeSent(timeSent);
    }

    u32 getSysConfigFileSentBytes() {
        return getSaveDataHandleSequence()->getSysConfigFile()->getSentBytes();
    }

    void setSysConfigFileSentBytes(u32 sentBytes) {
        getSaveDataHandleSequence()->getSysConfigFile()->setSentBytes(sentBytes);
    }








    s32 calcGreenStarNum(const GameDataHolder* pGameDataHolder) {
        s32 greenStarNum = 0;
        char eventFlagName[32];

        for (int i = 1; i <= 3; i++) {
            snprintf(eventFlagName, sizeof(eventFlagName), "SpecialStarGreen%1d", i);

            if (pGameDataHolder->isOnGameEventFlag(eventFlagName)) {
                greenStarNum++;
            }
        }

        return greenStarNum;
    }
};  // namespace GameDataFunction
#pragma dont_inline reset

#pragma dont_inline on
extern char lbl_806FF36C[];  // SMG2 event flag name (Shift-JIS "IsOpenScenario..." style), kept as data reference
extern char lbl_806FF388[];
extern char lbl_806FF3A0[];
extern char lbl_806FF3C0[];
extern char lbl_806FF3DC[];

namespace GameDataFunction {
    bool isOnGameEventFlagEnding120Stars() {
        const char* pFlagName = lbl_806FF36C;
        return GameDataFunction::getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(pFlagName);
    }

    bool isOnGameEventFlagFinalGalaxyAppearance() {
        const char* pFlagName = lbl_806FF388;
        return GameDataFunction::getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(pFlagName);
    }

    bool isOnGameEventFlagStarPieceCounterStop() {
        const char* pFlagName = lbl_806FF3A0;
        return GameDataFunction::getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(pFlagName);
    }

    bool isOnGameEventFlagCoinCounterStop() {
        const char* pFlagName = lbl_806FF3C0;
        return GameDataFunction::getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(pFlagName);
    }

    bool isOnGameEventFlagCometExplain() {
        const char* pFlagName = lbl_806FF3DC;
        return GameDataFunction::getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(pFlagName);
    }

    s32 calcCurrentGrandStarNum() {
        s32 grandStarNum = 0;

        for (s32 i = 1; i <= 6; i++) {
            if (GameDataFunction::getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->hasGrandStar(i)) {
                grandStarNum++;
            }
        }

        return grandStarNum;
    }
};  // namespace GameDataFunction
#pragma dont_inline reset
#pragma dont_inline reset
