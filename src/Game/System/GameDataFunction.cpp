#include "Game/System/GameDataFunction.hpp"
#include "Game/Util.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameDataGalaxyStorage.hpp"
#include "Game/System/GameDataHolder.hpp"
#include "Game/System/GameDataPlayerStatus.hpp"
#include "Game/System/GameDataWorldMapStorage.hpp"
#include "Game/System/GameEventFlagTable.hpp"
#include "Game/System/GameEventValueChecker.hpp"
#include "Game/System/GameSequenceDirector.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/SaveDataHandleSequence.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/System/GameDataTicoFatStorage.hpp"
#include "Game/System/SysConfigFile.hpp"
#include "Game/System/UserFile.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <cstdio>

// Callees that live in gap files of the SMG2 binary, not yet owned by a ported unit.
extern "C" {
    // PLACEHOLDER(core): fn_804DE190 is a GameDataPlayerStatus member in an unowned gap (stores mPlayerLeft into mUnknown14, sets mPlayerLeft to 4); resolve when its owner is ported
    void fn_804DE190(GameDataPlayerStatus*);
    // PLACEHOLDER(core): fn_804DEA50 reads a counter from the SysConfig chunk (SysConfigFile unit); resolve when SysConfigFile is ported
    s32 fn_804DEA50(SysConfigChunk*);
    // PLACEHOLDER(core): fn_804D88F0 is GameSequenceFunction-lane code (galaxy name, bool); resolve when GameSequenceFunction names it
    void fn_804D88F0(const char*, bool);
}

namespace GameDataFunction {
    SaveDataHandleSequence* getSaveDataHandleSequence() NO_INLINE {
        return SingletonHolder< GameSystem >::get()->mSequenceDirector->mSaveDataHandleSequence;
    }

    GameDataHolder* getCurrentGameDataHolder() {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder;
    }

    s32 getPlayerGameOverNum();

#pragma dont_inline on
    GameEventValueChecker* getCurrentGameEventValueChecker() {
        return getCurrentGameDataHolder()->mEventValueChecker;
    }

    GameDataPlayerStatus* getCurrentGameDataPlayerStatus() {
        return getCurrentGameDataHolder()->mPlayerStatus;
    }

    GameDataSomeGalaxyStorage* makeSomeGalaxyStorage(const char* pGalaxyName) {
        return getCurrentGameDataHolder()->mAllGalaxyStorage->makeAccessor(pGalaxyName);
    }

    GameDataTicoFatStorage* getCurrentGameDataTicoFatStorage() {
        return getCurrentGameDataHolder()->mTicoFatStorage;
    }

    GameDataWorldMapStorage* getCurrentGameDataWorldMapStorage() {
        return getCurrentGameDataHolder()->mWorldMapStorage;
    }

    void setPowerStar(const char* pGalaxyName, s32 scenarioNo) {
        makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mFlag |= 0x1;
        makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mFlag &= 0xFD;
    }

    bool hasPowerStar(const char* pGalaxyName, s32 scenarioNo) {
        return makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->testFlag(0x1);
    }

};  // namespace GameDataFunction

bool GameDataSomeScenarioAccessor::testFlag(u32 flag) const {
    u8 mask = flag;
    return (mFlag & mask) == mask;
}

namespace GameDataFunction {
    bool hasGrandStar(int id) {
        return getCurrentGameDataHolder()->hasGrandStar(id);
    }

    void setBronzeStar(const char* pGalaxyName, s32 scenarioNo) {
        makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mFlag |= 0x2;
        makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mFlag |= 0x1;
    }

    bool hasBronzeStar(const char* pGalaxyName, s32 scenarioNo) {
        return makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->testFlag(0x2);
    }

    s32 calcCurrentPowerStarNum() {
        return getCurrentGameDataHolder()->calcCurrentPowerStarNum();
    }

    s32 calcCurrentGrandStarNum() {
        s32 grandStarNum = 0;

        for (s32 i = 1; i <= 6; i++) {
            if (getCurrentGameDataHolder()->hasGrandStar(i)) {
                grandStarNum++;
            }
        }

        return grandStarNum;
    }

    s32 getNormalPowerStarNumOwned(const char*) {
        return getCurrentGameDataHolder()->calcNormalPowerStarNum();
    }

    s32 getPowerStarNumOwned(const char* pGalaxyName) {
        return getCurrentGameDataHolder()->mAllGalaxyStorage->getPowerStarNumOwned(pGalaxyName);
    }

    s32 calcNormalPowerStarNumFromWorld(s32 worldNo) {
        s32 powerStarNum = 0;

        for (ScenarioDataIter iter = MR::makeBeginScenarioDataIter(); !iter.isEnd(); iter.goNext()) {
            GalaxyStatusAccessor accessor = iter.makeAccessor();

            if (worldNo <= accessor.getWorldNo() && accessor.getWorldNo() <= 7) {
                powerStarNum += getPowerStarNumOwned(accessor.getName());
            }
        }

        return powerStarNum;
    }

    s32 getPowerStarNumMax() {
        s32 powerStarNumMax = 0;

        for (ScenarioDataIter iter = MR::makeBeginScenarioDataIter(); !iter.isEnd(); iter.goNext()) {
            powerStarNumMax += iter.makeAccessor().getPowerStarNum();
        }

        return powerStarNumMax;
    }

    void onGalaxyScenarioFlagGhostLuigi(const char* pGalaxyName, s32 scenarioNo) {
        makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mFlag |= 0x8;
    }

    bool isOnGalaxyScenarioFlagGhostLuigi(const char* pGalaxyName, s32 scenarioNo) {
        return makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->testFlag(0x8);
    }

    bool isOnGalaxyScenarioFlagAlreadyVisited(const char* pGalaxyName, s32 scenarioNo) {
        return makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->testFlag(0x4);
    }

    void onGalaxyScenarioFlagAlreadyVisited(const char* pGalaxyName, s32 scenarioNo) {
        makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mFlag |= 0x4;
    }

    bool isGalaxyStateClosed(const char* pGalaxyName) {
        return makeSomeGalaxyStorage(pGalaxyName)->mGalaxyState == 0;
    }

    bool isGalaxyStateNew(const char* pGalaxyName) {
        return makeSomeGalaxyStorage(pGalaxyName)->mGalaxyState == 1;
    }

    bool isGalaxyStateOpened(const char* pGalaxyName) {
        return makeSomeGalaxyStorage(pGalaxyName)->mGalaxyState == 2;
    }

    void setGalaxyStateNew(const char* pGalaxyName) {
        GameDataSomeGalaxyStorage* pStorage = makeSomeGalaxyStorage(pGalaxyName);
        pStorage->mGalaxyState = 1;
    }

    void setGalaxyStateOpened(const char* pGalaxyName) {
        GameDataSomeGalaxyStorage* pStorage = makeSomeGalaxyStorage(pGalaxyName);
        pStorage->mGalaxyState = 2;
    }

    void onGalaxyFlagTicoCoin(const char* pGalaxyName) {
        makeSomeGalaxyStorage(pGalaxyName)->mFlag |= 0x1;
    }

    bool isOnGalaxyFlagTicoCoin(const char* pGalaxyName) {
        return (makeSomeGalaxyStorage(pGalaxyName)->mFlag & 0x1) == 1;
    }

    s32 calcTicoCoinNum() {
        return getCurrentGameDataHolder()->calcTicoCoinNum();
    }

    void onGalaxyFlagComet(const char* pGalaxyName) {
        makeSomeGalaxyStorage(pGalaxyName)->mFlag |= 0x2;
    }

    void offGalaxyFlagComet(const char* pGalaxyName) {
        makeSomeGalaxyStorage(pGalaxyName)->mFlag &= 0xFD;
    }

    bool isOnGalaxyFlagComet(const char* pGalaxyName) {
        return (makeSomeGalaxyStorage(pGalaxyName)->mFlag & 0x2) == 2;
    }

    void tryOnDimensionBigCastleGalaxyComet() {
        if (!isOnGameEventFlagFinalGalaxyAppearance()) {
            return;
        }

        if (fn_804DEA50(getSaveDataHandleSequence()->getSysConfigFile()->mChunk) < 9999) {
            return;
        }

        GalaxyStatusAccessor accessor = MR::makeGalaxyStatusAccessor("DimensionBigCastleGalaxy");
        if (!accessor.hasPowerStar(1)) {
            return;
        }

        if (accessor.hasPowerStar(accessor.getCometScenarioId())) {
            return;
        }

        fn_804D88F0("DimensionBigCastleGalaxy", true);
    }

    void onGameEventFlag(const char* pName) {
        getCurrentGameDataHolder()->onGameEventFlag(pName);
    }

    void offGameEventFlag(const char* pName) {
        getCurrentGameDataHolder()->offGameEventFlag(pName);
    }

    bool isOnGameEventFlag(const char* pName) {
        return getCurrentGameDataHolder()->isOnGameEventFlag(pName);
    }

    bool isOnGameEventFlagEnding120Stars() {
        return getCurrentGameDataHolder()->isOnGameEventFlag("\203X\203^\201[120\214\302\203G\203\223\203f\203B\203\223\203O\216\300\215s");
    }

    bool isOnGameEventFlagFinalGalaxyAppearance() {
        return getCurrentGameDataHolder()->isOnGameEventFlag("\215\305\217I\203M\203\203\203\211\203N\203V\201[\217o\214\273");
    }

    bool isOnGameEventFlagStarPieceCounterStop() {
        return getCurrentGameDataHolder()->isOnGameEventFlag("\203X\203^\201[\203s\201[\203X\203J\203E\203\223\203^\201[\203X\203g\203b\203v");
    }

    bool isOnGameEventFlagCoinCounterStop() {
        return getCurrentGameDataHolder()->isOnGameEventFlag("\203R\203C\203\223\203J\203E\203\223\203^\201[\203X\203g\203b\203v");
    }

    bool isOnGameEventFlagCometExplain() {
        return getCurrentGameDataHolder()->isOnGameEventFlag("\203R\203\201\203b\203g\211\360\220\340");
    }

    void onGameEventFlagOpenScenario(const char* pGalaxyName, s32 scenarioNo) {
        char eventFlagName[0x80];
        snprintf(eventFlagName, sizeof(eventFlagName), "IsOpenScenario%s%01d", pGalaxyName, scenarioNo);
        getCurrentGameDataHolder()->onGameEventFlag(eventFlagName);
    }

    bool isOnGameEventFlagOpenScenario(const char* pGalaxyName, s32 scenarioNo) {
        char eventFlagName[0x80];
        snprintf(eventFlagName, sizeof(eventFlagName), "IsOpenScenario%s%01d", pGalaxyName, scenarioNo);
        return getCurrentGameDataHolder()->isOnGameEventFlag(eventFlagName);
    }

    bool existsGameEventFlagOpenScenario(const char* pGalaxyName, s32 scenarioNo) {
        char eventFlagName[0x80];
        snprintf(eventFlagName, sizeof(eventFlagName), "IsOpenScenario%s%01d", pGalaxyName, scenarioNo);
        return GameEventFlagTable::isExist(eventFlagName);
    }

    void forceLuigiPlayer() {
        getCurrentGameDataPlayerStatus()->mFlag |= 0x1;
        getCurrentGameDataHolder()->onGameEventFlag("\203\213\203C\201[\203W\203v\203\214\203C\215\317");
    }

    void forceMarioPlayer() {
        getCurrentGameDataPlayerStatus()->mFlag &= 0xFE;
    }

    bool isLuigiPlayer() {
        return (getCurrentGameDataPlayerStatus()->mFlag & 0x1) == 1;
    }

    void startLuigiIntrusion() {
        getCurrentGameDataHolder()->onGameEventFlag("\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\212J\216n");
        getCurrentGameDataHolder()->onGameEventFlag("\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\222\206");
    }

    bool isOnLuigiIntrusivelyStart() {
        return getCurrentGameDataHolder()->isOnGameEventFlag("\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\212J\216n");
    }

    void onLuigiIntrusivelyAppear() {
        if (!isOnCurrentScenarioFlagLuigiIntruded()) {
            s32 scenarioNo = MR::getCurrentScenarioNo();
            makeSomeGalaxyStorage(MR::getCurrentStageName())->getScenarioAccessor(scenarioNo)->mFlag |= 0x10;
            getCurrentGameDataHolder()->offGameEventFlag("\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\222\206");
        }
    }

    bool isOnCurrentScenarioFlagLuigiIntruded() {
        s32 scenarioNo = MR::getCurrentScenarioNo();
        return makeSomeGalaxyStorage(MR::getCurrentStageName())->getScenarioAccessor(scenarioNo)->testFlag(0x10);
    }

    void updateLuigiIntrusionCounter() {
        if (isOnLuigiIntrusivelyStart()) {
            if (!getCurrentGameDataHolder()->isOnGameEventFlag("\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\222\206")) {
                s32 counter = MR::clamp(static_cast< s32 >(static_cast< u16 >(getCurrentGameDataHolder()->getGameEventValue("\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\203J\203E\203\223\203^")) + 1), 0L, 0xFFFFL);

                if (counter % 6 == 0) {
                    getCurrentGameDataHolder()->onGameEventFlag("\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\222\206");
                    counter = 0;
                }

                getCurrentGameDataHolder()->setGameEventValue("\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\203J\203E\203\223\203^", static_cast< u16 >(counter));
            }
        }
    }

    bool isOnMessageAlreadyRead(s8 messageNo) {
        return getCurrentGameEventValueChecker()->isOnIndexedBit("\203\201\203b\203Z\201[\203W\212\371\223\307\203t\203\211\203O", messageNo);
    }

    void setOnMessageAlreadyRead(s8 messageNo) {
        getCurrentGameEventValueChecker()->onIndexedBit("\203\201\203b\203Z\201[\203W\212\371\223\307\203t\203\211\203O", messageNo);
    }

    bool isOnPostmanTaskLetterRead(s32 letterNo) {
        return getCurrentGameEventValueChecker()->isOnIndexedBit("\227X\225\326\211\256[\203^\203X\203N\216\350\216\206\212\371\223\307\203t\203\211\203O]", letterNo);
    }

    void setOnPostmanTaskLetterRead(s8 letterNo) {
        getCurrentGameEventValueChecker()->onIndexedBit("\227X\225\326\211\256[\203^\203X\203N\216\350\216\206\212\371\223\307\203t\203\211\203O]", letterNo);
        getCurrentGameEventValueChecker()->setValue("\227X\225\326\211\256[\215\305\214\343\202\311\223\307\202\361\202\276\203^\203X\203N\216\350\216\206\203C\203\223\203f\203b\203N\203X]", static_cast< u16 >(letterNo));
    }

    bool isOnPostmanImportantLetterRead(s32 letterNo) {
        return getCurrentGameEventValueChecker()->isOnIndexedBit("\227X\225\326\211\256[\217d\227v\216\350\216\206\212\371\223\307\203t\203\211\203O]", letterNo);
    }

    void setOnPostmanImportantLetterRead(s8 letterNo) {
        getCurrentGameEventValueChecker()->onIndexedBit("\227X\225\326\211\256[\217d\227v\216\350\216\206\212\371\223\307\203t\203\211\203O]", letterNo);
    }

    void updatePostmanPeachLetterDeath() {
        s32 playerMissNum = getCurrentGameDataHolder()->getPlayerMissNum();
        getCurrentGameEventValueChecker()->setValue("\227X\225\326\211\256[\203s\201[\203`\216\350\216\206\202\360\223\307\202\361\202\276\216\236\202\314\227\335\220\317\216\200\226S\211\361\220\224]", static_cast< u16 >(playerMissNum));
    }

    s32 getBankInterest() {
        return MR::clamp(static_cast< s32 >(static_cast< u16 >(getCurrentGameDataHolder()->getGameEventValue("\213\342\215s\211\256\203L\203m\203s\203I[\227\230\216q]"))), 0L, 0x270FL);
    }

    void updateBankInterest() {
        s32 interest = MR::clamp(static_cast< s32 >(GameSequenceFunction::getClearedStarPieceNum() % 10), 0L, 0x270FL);
        getCurrentGameDataHolder()->setGameEventValue("\213\342\215s\211\256\203L\203m\203s\203I[\227\230\216q]", static_cast< u16 >(interest));
    }

    void clearKinopioBankInterest() {
        getCurrentGameDataHolder()->setGameEventValue("\213\342\215s\211\256\203L\203m\203s\203I[\227\230\216q]", 0);
    }

    void setRaceBestTime(const char* pRaceName, u32 bestTime) {
        getCurrentGameEventValueChecker()->setU32Value(pRaceName, bestTime);
    }

    s32 getRaceBestTime(const char* pRaceName) {
        return getCurrentGameEventValueChecker()->getU32Value(pRaceName);
    }

    void setBestScoreAttackCurrentStage(u32 score) {
        char eventFlagName[0x80];
        snprintf(eventFlagName, sizeof(eventFlagName), "\203x\203X\203g\203X\203R\203A[%s]", MR::getCurrentStageName());

        if (score > 999999) {
            score = 999999;
        }

        getCurrentGameEventValueChecker()->setU32Value(eventFlagName, score);
    }

    u32 getBestScoreAttackCurrentStage(const char* pStageName) {
        if (pStageName == nullptr) {
            pStageName = MR::getCurrentStageName();
        }

        char eventFlagName[0x80];
        snprintf(eventFlagName, sizeof(eventFlagName), "\203x\203X\203g\203X\203R\203A[%s]", pStageName);

        u32 score = getCurrentGameEventValueChecker()->getU32Value(eventFlagName);
        if (score > 999999) {
            score = 999999;
        }

        return score;
    }

    void incPlayerMissNum() {
        getCurrentGameDataHolder()->incPlayerMissNum();
    }

    s32 getPlayerMissNum() {
        return getCurrentGameDataHolder()->getPlayerMissNum();
    }

    void incScenarioMissNum(const char* pGalaxyName, s32 scenarioNo) {
        s32 missNum = MR::clamp(static_cast< s32 >(makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mMissNum), 0L, 0x63L);
        missNum = MR::clamp(missNum + 1, 0L, 0x63L);
        makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mMissNum = missNum;
    }

    s32 getScenarioMissNum(const char* pGalaxyName, s32 scenarioNo) {
        return MR::clamp(static_cast< s32 >(makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mMissNum), 0L, 0x63L);
    }

    void incPlayerGameOverNum() {
        s32 gameOverNum = MR::clamp(getPlayerGameOverNum() + 1, 0L, 0xFFFFL);
        getCurrentGameDataHolder()->setGameEventValue("\227\335\220\317\203Q\201[\203\200\203I\201[\203o\201[\211\361\220\224", static_cast< u16 >(gameOverNum));
    }

    s32 getPlayerGameOverNum() {
        return MR::clamp(static_cast< s32 >(static_cast< u16 >(getCurrentGameDataHolder()->getGameEventValue("\227\335\220\317\203Q\201[\203\200\203I\201[\203o\201[\211\361\220\224"))), 0L, 0xFFFFL);
    }

    // SMG2 body is empty (blr).
    void updateStorageSpinDriverPathDrawRange(const char*, int, int, f32) {
    }

    s32 getTicoFatStarPieceAlms(s32 index) {
        GameDataWorldMapStorage* pWorldMap = getCurrentGameDataWorldMapStorage();
        GameDataTicoFatStorage* pTicoFat = getCurrentGameDataTicoFatStorage();
        return pTicoFat->getValue(index + (pWorldMap->mCurrentWorldNo - 1) * 6);
    }

    s32 getTicoFatStarPieceAlmsFromWorld(s32 worldNo, s32 index) {
        GameDataTicoFatStorage* pTicoFat = getCurrentGameDataTicoFatStorage();
        return pTicoFat->getValue(index + (worldNo - 1) * 6);
    }

    void addTicoFatStarPieceAlms(s32 index, s32 value) {
        GameDataWorldMapStorage* pWorldMap = getCurrentGameDataWorldMapStorage();
        GameDataTicoFatStorage* pTicoFat = getCurrentGameDataTicoFatStorage();
        pTicoFat->addValue(index + (pWorldMap->mCurrentWorldNo - 1) * 6, value);
    }

    void onTicoCoinFeed(const char* pName) {
        getCurrentGameDataTicoFatStorage()->onTicoCoinFeed(pName);
    }

    bool isTicoCoinFeed(const char* pName) {
        return getCurrentGameDataTicoFatStorage()->isTicoCoinFeed(pName);
    }

    s32 getPlayerLeft() {
        return getCurrentGameDataPlayerStatus()->getPlayerLeft();
    }

    void addPlayerLeft(int num) {
        getCurrentGameDataPlayerStatus()->addPlayerLeft(num);
    }

    s32 getPlayerStatusCounter14() {
        return getCurrentGameDataPlayerStatus()->mUnknown14;
    }

    s32 getStockedStarPieceNum() {
        return getCurrentGameDataHolder()->getStocketStarPieceNum();
    }

    void tryOnGameEventFlagStarPieceCounterStop(int num) {
        getCurrentGameDataHolder()->addStockedStarPieceNum(num);

        if (getCurrentGameDataHolder()->getStocketStarPieceNum() >= 9999) {
            if (!isOnGameEventFlagStarPieceCounterStop()) {
                getCurrentGameDataHolder()->onGameEventFlag("\203X\203^\201[\203s\201[\203X\203J\203E\203\223\203^\201[\203X\203g\203b\203v");
            }
        }
    }

    s32 getStockedCoinNum() {
        return getCurrentGameDataPlayerStatus()->getStockedCoinNum();
    }

    s32 tryOnGameEventFlagCoinCounterStop(int num) {
        s32 stockedCoinNum = getCurrentGameDataPlayerStatus()->addStockedCoinNum(num);

        if (getCurrentGameDataPlayerStatus()->getStockedCoinNum() >= 9999) {
            if (!isOnGameEventFlagCoinCounterStop()) {
                getCurrentGameDataHolder()->onGameEventFlag("\203R\203C\203\223\203J\203E\203\223\203^\201[\203X\203g\203b\203v");
            }
        }

        return stockedCoinNum;
    }

    void setScenarioBestTime(const char* pGalaxyName, s32 scenarioNo, s32 bestTime) {
        s32 clampedTime = bestTime;
        if (bestTime >= 216000) {
            clampedTime = 216000;
        }

        makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mBestTime = clampedTime;
    }

    u32 getScenarioClearTime(const char* pGalaxyName, s32 scenarioNo) {
        s32 bestTime = makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mBestTime;
        if (bestTime >= 216000) {
            return 216000;
        }

        return bestTime;
    }

    void incCumulativePlayTime() {
        getCurrentGameDataHolder()->incCumulativePlayTime();
    }

    s32 getCumulativePlayTime() {
        return getCurrentGameDataHolder()->getCumulativePlayTime();
    }

    void resetAllGameData() {
        getCurrentGameDataHolder()->resetAllData();
    }

    void resetPlayerLeftAndTryOnDimensionBigCastleGalaxyComet() {
        fn_804DE190(getCurrentGameDataPlayerStatus());
        tryOnDimensionBigCastleGalaxyComet();
    }

    bool isOnWorldMapPointFlag(s32 index, s32 bit) {
        return getCurrentGameDataWorldMapStorage()->isOnPointFlag(index, bit);
    }

    void onWorldMapPointFlag(s32 index, s32 bit) {
        getCurrentGameDataWorldMapStorage()->setPointFlag(index, bit, true);
    }

    void offWorldMapPointFlag(s32 index, s32 bit) {
        getCurrentGameDataWorldMapStorage()->setPointFlag(index, bit, false);
    }

    void setCurrentWorldId(u8 worldNo) {
        getCurrentGameDataWorldMapStorage()->setCurrentWorldNo(worldNo);
    }

    s32 getCurrentWorldId() {
        return getCurrentGameDataWorldMapStorage()->mCurrentWorldNo;
    }

    s32 calcLastEnteredWorldNo() {
        s32 worldNo = 7;

        while (true) {
            char eventFlagName[0x40];
            snprintf(eventFlagName, sizeof(eventFlagName), "\203\217\201[\203\213\203h%d\217\211\203v\203\214\203C", worldNo);

            if (getCurrentGameDataHolder()->isOnGameEventFlag(eventFlagName)) {
                break;
            }

            worldNo--;
            if (worldNo <= 1) {
                break;
            }
        }

        if (getCurrentGameDataWorldMapStorage()->mCurrentWorldNo > worldNo) {
            worldNo = getCurrentGameDataWorldMapStorage()->mCurrentWorldNo;
        }

        return worldNo;
    }

    bool isOnGameEventFlagWorld1Entered() {
        return getCurrentGameDataHolder()->isOnGameEventFlag("\203\217\201[\203\213\203h1\217\211\203v\203\214\203C");
    }

    s32 calcFaceShipEventNum() {
        s32 eventNum = 0;

        for (s32 i = 0; i < 4; i++) {
            if (static_cast< u16 >(getCurrentGameEventValueChecker()->getIndexedValue("\212\347\230f\220\257\203C\203x\203\223\203g\224\324\215\206", i)) > 0) {
                eventNum++;
            }
        }

        return eventNum;
    }

    s32 getFaceShipEventNo(s32 index) {
        return static_cast< u16 >(getCurrentGameEventValueChecker()->getIndexedValue("\212\347\230f\220\257\203C\203x\203\223\203g\224\324\215\206", index));
    }

    void tryAddFaceShipEventNo(s32 eventNo) {
        for (s32 i = 0; i < 4; i++) {
            if (static_cast< u16 >(getCurrentGameEventValueChecker()->getIndexedValue("\212\347\230f\220\257\203C\203x\203\223\203g\224\324\215\206", i)) <= 0) {
                getCurrentGameEventValueChecker()->setIndexedValue("\212\347\230f\220\257\203C\203x\203\223\203g\224\324\215\206", i, static_cast< u16 >(eventNo));
                return;
            }
        }
    }

    s32 getFaceShipEventGrandStarNo() {
        u16 grandStarNo = getCurrentGameEventValueChecker()->getValue("\212\347\230f\220\257\203C\203x\203\223\203g\203O\203\211\203\223\203h\203X\203^\201[\224\324\215\206");

        if (grandStarNo != 0) {
            return MR::clamp(grandStarNo, 1L, 5L);
        }

        return -1;
    }

    void setFaceShipEventGrandStarNo(s32 grandStarNo) {
        getCurrentGameEventValueChecker()->setValue("\212\347\230f\220\257\203C\203x\203\223\203g\203O\203\211\203\223\203h\203X\203^\201[\224\324\215\206", static_cast< u16 >(grandStarNo));
    }

    void clearFaceShipEventValues() {
        for (s32 i = 0; i < 4; i++) {
            getCurrentGameEventValueChecker()->setIndexedValue("\212\347\230f\220\257\203C\203x\203\223\203g\224\324\215\206", i, 0);
        }

        getCurrentGameEventValueChecker()->setValue("\212\347\230f\220\257\203C\203x\203\223\203g\203O\203\211\203\223\203h\203X\203^\201[\224\324\215\206", 0);
    }

    u16 getStageClearsAfterRepeatedMissCount() {
        return getCurrentGameEventValueChecker()->getValue("\210\352\222\350\220\224\216\200\226S\214\343\202\314\203X\203e\201[\203W\203N\203\212\203A\211\361\220\224");
    }

    void setStageClearsAfterRepeatedMissCount(u16 count) {
        getCurrentGameEventValueChecker()->setValue("\210\352\222\350\220\224\216\200\226S\214\343\202\314\203X\203e\201[\203W\203N\203\212\203A\211\361\220\224", count);
    }

    void incStageClearsAfterRepeatedMissCount() {
        u32 count = getCurrentGameEventValueChecker()->getValue("\210\352\222\350\220\224\216\200\226S\214\343\202\314\203X\203e\201[\203W\203N\203\212\203A\211\361\220\224");
        getCurrentGameEventValueChecker()->setValue("\210\352\222\350\220\224\216\200\226S\214\343\202\314\203X\203e\201[\203W\203N\203\212\203A\211\361\220\224", static_cast< u16 >(count + 1));
    }

    const wchar_t* getUserName() {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mUserName;
    }

    void setUserName(wchar_t* const pUserName) {
        getSaveDataHandleSequence()->getCurrentUserFile()->setUserName(pUserName);
    }

    bool hasCurrentUserMiiId() {
        u32 miiId[2];
        return getSaveDataHandleSequence()->getCurrentUserFile()->getMiiId(miiId);
    }

    bool getSysConfigFileEncouragedPal16() {
        return getSaveDataHandleSequence()->getSysConfigFile()->mChunk->mEncouragedPal16;
    }

    void setSysConfigFileEncouragedPal16() {
        getSaveDataHandleSequence()->getSysConfigFile()->mChunk->mEncouragedPal16 = 1;
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
#pragma dont_inline reset
};  // namespace GameDataFunction
