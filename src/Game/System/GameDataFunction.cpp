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

namespace GameDataFunction {
    SaveDataHandleSequence* getSaveDataHandleSequence() NO_INLINE {
        return SingletonHolder< GameSystem >::get()->mSequenceDirector->mSaveDataHandleSequence;
    }


    s32 getPlayerGameOverNum();



#pragma dont_inline on
    GameEventValueChecker* getCurrentGameEventValueChecker() {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->mEventValueChecker;
    }

    GameDataPlayerStatus* getCurrentGameDataPlayerStatus() {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->mPlayerStatus;
    }

    GameDataSomeGalaxyStorage* makeSomeGalaxyStorage(const char* pGalaxyName) {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->mAllGalaxyStorage->makeAccessor(pGalaxyName);
    }

    GameDataTicoFatStorage* getCurrentGameDataTicoFatStorage() {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->mTicoFatStorage;
    }

    GameDataWorldMapStorage* getCurrentGameDataWorldMapStorage() {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->mWorldMapStorage;
    }

    void setPowerStar(const char* pGalaxyName, s32 scenarioNo) {
        makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mFlag |= 0x1;
        makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mFlag &= 0xFD;
    }

    bool hasPowerStar(const char* pGalaxyName, s32 scenarioNo) {
        return makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->testFlag(0x1);
    }

};  // namespace GameDataFunction

// NONMATCHING: regalloc; AND operands are reversed.
bool GameDataSomeScenarioAccessor::testFlag(u32 flag) const {
    u8 mask = flag;
    return (mFlag & mask) == mask;
}

namespace GameDataFunction {
    bool hasGrandStar(int id) {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->hasGrandStar(id);
    }

    void setBronzeStar(const char* pGalaxyName, s32 scenarioNo) {
        makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mFlag |= 0x2;
        makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->mFlag |= 0x1;
    }

    bool hasBronzeStar(const char* pGalaxyName, s32 scenarioNo) {
        return makeSomeGalaxyStorage(pGalaxyName)->getScenarioAccessor(scenarioNo)->testFlag(0x2);
    }

    s32 calcCurrentPowerStarNum() {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->calcCurrentPowerStarNum();
    }

    s32 calcCurrentGrandStarNum() {
        s32 grandStarNum = 0;

        for (s32 i = 1; i <= 6; i++) {
            if (getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->hasGrandStar(i)) {
                grandStarNum++;
            }
        }

        return grandStarNum;
    }

    s32 getNormalPowerStarNumOwned() {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->calcNormalPowerStarNum();
    }

    s32 getPowerStarNumOwned(const char* pGalaxyName) {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->mAllGalaxyStorage->getPowerStarNumOwned(pGalaxyName);
    }

#pragma dont_inline off
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

#pragma dont_inline on
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

    // NONMATCHING: regalloc; equivalent bit test is optimized to a rotate.
    bool isOnGalaxyFlagTicoCoin(const char* pGalaxyName) {
        return (makeSomeGalaxyStorage(pGalaxyName)->mFlag & 0x1) == 1;
    }

    s32 calcTicoCoinNum() {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->calcTicoCoinNum();
    }

    void onGalaxyFlagComet(const char* pGalaxyName) {
        makeSomeGalaxyStorage(pGalaxyName)->mFlag |= 0x2;
    }

    void offGalaxyFlagComet(const char* pGalaxyName) {
        makeSomeGalaxyStorage(pGalaxyName)->mFlag &= 0xFD;
    }

    // NONMATCHING: regalloc; equivalent bit test is optimized to a rotate.
    bool isOnGalaxyFlagComet(const char* pGalaxyName) {
        return (makeSomeGalaxyStorage(pGalaxyName)->mFlag & 0x2) == 2;
    }

#pragma dont_inline off
    // NONMATCHING: regalloc; string address materialization order.
    void tryOnDimensionBigCastleGalaxyComet() {
        if (!isOnGameEventFlagFinalGalaxyAppearance()) {
            return;
        }

        if (getSaveDataHandleSequence()->getSysConfigFile()->mChunk->getMaxBankedStarPieceNum() < 9999) {
            return;
        }

        GalaxyStatusAccessor accessor = MR::makeGalaxyStatusAccessor("DimensionBigCastleGalaxy");
        if (!accessor.hasPowerStar(1)) {
            return;
        }

        if (accessor.hasPowerStar(accessor.getCometScenarioId())) {
            return;
        }

        GameSequenceFunction::updateGalaxyCometAppearance("DimensionBigCastleGalaxy", true);
    }

#pragma dont_inline on
    void onGameEventFlag(const char* pName) {
        getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->onGameEventFlag(pName);
    }

    void offGameEventFlag(const char* pName) {
        getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->offGameEventFlag(pName);
    }

    bool isOnGameEventFlag(const char* pName) {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(pName);
    }

    bool isOnGameEventFlagEnding120Stars() {
        const char* eventName1 = "\203X\203^\201[120\214\302\203G\203\223\203f\203B\203\223\203O\216\300\215s";
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(eventName1);
    }

    bool isOnGameEventFlagFinalGalaxyAppearance() {
        const char* eventName2 = "\215\305\217I\203M\203\203\203\211\203N\203V\201[\217o\214\273";
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(eventName2);
    }

    bool isOnGameEventFlagStarPieceCounterStop() {
        const char* eventName3 = "\203X\203^\201[\203s\201[\203X\203J\203E\203\223\203^\201[\203X\203g\203b\203v";
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(eventName3);
    }

    bool isOnGameEventFlagCoinCounterStop() {
        const char* eventName4 = "\203R\203C\203\223\203J\203E\203\223\203^\201[\203X\203g\203b\203v";
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(eventName4);
    }

    bool isOnGameEventFlagCometExplain() {
        const char* eventName5 = "\203R\203\201\203b\203g\211\360\220\340";
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(eventName5);
    }

    void onGameEventFlagOpenScenario(const char* pGalaxyName, s32 scenarioNo) {
        char eventFlagName[0x80];
        snprintf(eventFlagName, sizeof(eventFlagName), "IsOpenScenario%s%01d", pGalaxyName, scenarioNo);
        getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->onGameEventFlag(eventFlagName);
    }

    bool isOnGameEventFlagOpenScenario(const char* pGalaxyName, s32 scenarioNo) {
        char eventFlagName[0x80];
        snprintf(eventFlagName, sizeof(eventFlagName), "IsOpenScenario%s%01d", pGalaxyName, scenarioNo);
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(eventFlagName);
    }

    bool existsGameEventFlagOpenScenario(const char* pGalaxyName, s32 scenarioNo) {
        char eventFlagName[0x80];
        snprintf(eventFlagName, sizeof(eventFlagName), "IsOpenScenario%s%01d", pGalaxyName, scenarioNo);
        return GameEventFlagTable::isExist(eventFlagName);
    }

    void forceLuigiPlayer() {
        getCurrentGameDataPlayerStatus()->mFlag |= 0x1;
        const char* eventName6 = "\203\213\203C\201[\203W\203v\203\214\203C\215\317";
        getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->onGameEventFlag(eventName6);
    }

    void forceMarioPlayer() {
        getCurrentGameDataPlayerStatus()->mFlag &= 0xFE;
    }

    // NONMATCHING: regalloc; equivalent bit test is optimized to a rotate.
    bool isLuigiPlayer() {
        return (getCurrentGameDataPlayerStatus()->mFlag & 0x1) == 1;
    }

    void startLuigiIntrusion() {
        const char* eventName7 = "\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\212J\216n";
        getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->onGameEventFlag(eventName7);
        const char* eventName8 = "\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\222\206";
        getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->onGameEventFlag(eventName8);
    }

    bool isOnLuigiIntrusivelyStart() {
        const char* eventName9 = "\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\212J\216n";
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(eventName9);
    }

    void onLuigiIntrusivelyAppear() {
        if (!isOnCurrentScenarioFlagLuigiIntruded()) {
            s32 scenarioNo = MR::getCurrentScenarioNo();
            makeSomeGalaxyStorage(MR::getCurrentStageName())->getScenarioAccessor(scenarioNo)->mFlag |= 0x10;
            const char* eventName10 = "\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\222\206";
            getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->offGameEventFlag(eventName10);
        }
    }

    bool isOnCurrentScenarioFlagLuigiIntruded() {
        s32 scenarioNo = MR::getCurrentScenarioNo();
        return makeSomeGalaxyStorage(MR::getCurrentStageName())->getScenarioAccessor(scenarioNo)->testFlag(0x10);
    }

    // NONMATCHING: regalloc; preserved registers and stack layout.
    void updateLuigiIntrusionCounter() {
        if (isOnLuigiIntrusivelyStart()) {
            const char* eventName11 = "\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\222\206";
            if (!getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(eventName11)) {
                const char* eventName12 = "\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\203J\203E\203\223\203^";
                s32 counter = MR::clamp(static_cast< s32 >(static_cast< u16 >(getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->getGameEventValue(eventName12)) + 1), 0L, 0xFFFFL);

                if (counter % 6 == 0) {
                    const char* eventName13 = "\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\222\206";
                    getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->onGameEventFlag(eventName13);
                    counter = 0;
                }

                const char* eventName14 = "\202\305\202\265\202\341\202\316\202\350\203\213\203C\201[\203W\217o\214\273\203J\203E\203\223\203^";
                getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->setGameEventValue(eventName14, static_cast< u16 >(counter));
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

    void setOnPostmanTaskLetterRead(s32 letterNo) {
        getCurrentGameEventValueChecker()->onIndexedBit("\227X\225\326\211\256[\203^\203X\203N\216\350\216\206\212\371\223\307\203t\203\211\203O]", letterNo);
        getCurrentGameEventValueChecker()->setValue("\227X\225\326\211\256[\215\305\214\343\202\311\223\307\202\361\202\276\203^\203X\203N\216\350\216\206\203C\203\223\203f\203b\203N\203X]", static_cast< u16 >(letterNo));
    }

    bool isOnPostmanImportantLetterRead(s32 letterNo) {
        return getCurrentGameEventValueChecker()->isOnIndexedBit("\227X\225\326\211\256[\217d\227v\216\350\216\206\212\371\223\307\203t\203\211\203O]", letterNo);
    }

    void setOnPostmanImportantLetterRead(s32 letterNo) {
        getCurrentGameEventValueChecker()->onIndexedBit("\227X\225\326\211\256[\217d\227v\216\350\216\206\212\371\223\307\203t\203\211\203O]", letterNo);
    }

    void updatePostmanPeachLetterDeath() {
        s32 playerMissNum = getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->getPlayerMissNum();
        getCurrentGameEventValueChecker()->setValue("\227X\225\326\211\256[\203s\201[\203`\216\350\216\206\202\360\223\307\202\361\202\276\216\236\202\314\227\335\220\317\216\200\226S\211\361\220\224]", static_cast< u16 >(playerMissNum));
    }

    s32 getBankInterest() {
        const char* eventName15 = "\213\342\215s\211\256\203L\203m\203s\203I[\227\230\216q]";
        return MR::clamp(static_cast< s32 >(static_cast< u16 >(getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->getGameEventValue(eventName15))), 0L, 0x270FL);
    }

    void updateBankInterest() {
        s32 interest = MR::clamp(static_cast< s32 >(GameSequenceFunction::getClearedStarPieceNum() % 10), 0L, 0x270FL);
        const char* eventName16 = "\213\342\215s\211\256\203L\203m\203s\203I[\227\230\216q]";
        getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->setGameEventValue(eventName16, static_cast< u16 >(interest));
    }

    void clearKinopioBankInterest() {
        const char* eventName17 = "\213\342\215s\211\256\203L\203m\203s\203I[\227\230\216q]";
        getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->setGameEventValue(eventName17, 0);
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

    // NONMATCHING: regalloc; equivalent null-argument branch arrangement.
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
        getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->incPlayerMissNum();
    }

    s32 getPlayerMissNum() {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->getPlayerMissNum();
    }

    // NONMATCHING: regalloc; redundant byte narrowing before the store.
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
        const char* eventName18 = "\227\335\220\317\203Q\201[\203\200\203I\201[\203o\201[\211\361\220\224";
        getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->setGameEventValue(eventName18, static_cast< u16 >(gameOverNum));
    }

    s32 getPlayerGameOverNum() {
        const char* eventName19 = "\227\335\220\317\203Q\201[\203\200\203I\201[\203o\201[\211\361\220\224";
        return MR::clamp(static_cast< s32 >(static_cast< u16 >(getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->getGameEventValue(eventName19))), 0L, 0xFFFFL);
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
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->getStocketStarPieceNum();
    }

    void tryOnGameEventFlagStarPieceCounterStop(int num) {
        getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->addStockedStarPieceNum(num);

        if (getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->getStocketStarPieceNum() >= 9999) {
            if (!isOnGameEventFlagStarPieceCounterStop()) {
                const char* eventName20 = "\203X\203^\201[\203s\201[\203X\203J\203E\203\223\203^\201[\203X\203g\203b\203v";
                getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->onGameEventFlag(eventName20);
            }
        }
    }

    s32 getStockedCoinNum() {
        return getCurrentGameDataPlayerStatus()->getStockedCoinNum();
    }

    // NONMATCHING: regalloc; preserved registers.
    s32 tryOnGameEventFlagCoinCounterStop(int num) {
        s32 stockedCoinNum = getCurrentGameDataPlayerStatus()->addStockedCoinNum(num);

        if (getCurrentGameDataPlayerStatus()->getStockedCoinNum() >= 9999) {
            if (!isOnGameEventFlagCoinCounterStop()) {
                const char* eventName21 = "\203R\203C\203\223\203J\203E\203\223\203^\201[\203X\203g\203b\203v";
                getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->onGameEventFlag(eventName21);
            }
        }

        return stockedCoinNum;
    }

    void setScenarioBestTime(const char* pGalaxyName, s32 scenarioNo, s32 bestTime) {
        s32 clampedTime = 216000;
        if (bestTime < clampedTime) {
            clampedTime = bestTime;
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
        getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->incCumulativePlayTime();
    }

    s32 getCumulativePlayTime() {
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->getCumulativePlayTime();
    }

    void resetAllGameData() {
        getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->resetAllData();
    }

    void resetPlayerLeftAndTryOnDimensionBigCastleGalaxyComet() {
        getCurrentGameDataPlayerStatus()->resetPlayerLeft();
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

            if (getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(eventFlagName)) {
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
        const char* eventName22 = "\203\217\201[\203\213\203h1\217\211\203v\203\214\203C";
        return getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->isOnGameEventFlag(eventName22);
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

        if (grandStarNo == 0) {
            return -1;
        }
        return MR::clamp(grandStarNo, 1L, 5L);
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

    u32 getStageClearsAfterRepeatedMissCount() {
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

    void setUserName(const wchar_t* pUserName) {
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
        SysConfigChunk* chunk = getSaveDataHandleSequence()->getSysConfigFile()->mChunk;
        chunk->mEncouragedPal16 = true;
    }

    OSTime getSysConfigFileTimeSent() {
        return getSaveDataHandleSequence()->getSysConfigFile()->mChunk->mTimeSent;
    }

    void setSysConfigFileTimeSent(OSTime timeSent) {
        getSaveDataHandleSequence()->getSysConfigFile()->mChunk->mTimeSent = timeSent;
    }

    u32 getSysConfigFileSentBytes() {
        return getSaveDataHandleSequence()->getSysConfigFile()->mChunk->mSentBytes;
    }

    void setSysConfigFileSentBytes(u32 sentBytes) {
        getSaveDataHandleSequence()->getSysConfigFile()->mChunk->mSentBytes = sentBytes;
    }


    s32 getPendingSharedPlayerLeft() {
        u16 hash = MR::getHashCode(getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->mName);
        if (hash == getSaveDataHandleSequence()->getSysConfigFile()->mChunk->mCurrentCounters->mPlayerLeftHash) {
            return 0;
        }
        return getSaveDataHandleSequence()->getSysConfigFile()->mChunk->getSharedPlayerLeft();
    }

    void storeSharedPlayerLeft() {
        u16 hash = MR::getHashCode(getSaveDataHandleSequence()->getCurrentUserFile()->mGameDataHolder->mName);
        s32 left = getCurrentGameDataPlayerStatus()->getPlayerLeft();
        getSaveDataHandleSequence()->getSysConfigFile()->mChunk->setSharedPlayerLeft(left, hash);
    }

    s32 getBankedStarPieceNum() {
        return getSaveDataHandleSequence()->getSysConfigFile()->mChunk->getBankedStarPieceNum();
    }

    void setBankedStarPieceNum(s32 count) {
        getSaveDataHandleSequence()->getSysConfigFile()->mChunk->setBankedStarPieceNum(count);
        tryOnDimensionBigCastleGalaxyComet();
    }

    s32 getMaxBankedStarPieceNum() {
        return getSaveDataHandleSequence()->getSysConfigFile()->mChunk->getMaxBankedStarPieceNum();
    }

    bool isSysConfigCounterMax() {
        return getSaveDataHandleSequence()->getSysConfigFile()->mChunk->getMaxBankedStarPieceNum() >= 9999;
    }

    void resetSysConfigCounters() {
        getSaveDataHandleSequence()->getSysConfigFile()->mChunk->resetCounters();
    }

    void copySysConfigData() {
        getSaveDataHandleSequence()->getSysConfigFile()->mChunk->restoreCounters();
    }

};  // namespace GameDataFunction

#pragma dont_inline reset
