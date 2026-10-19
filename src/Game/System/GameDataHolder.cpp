#include "Game/System/GameDataHolder.hpp"
#include "Game/System/BinaryDataChunkHolder.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameDataGalaxyStorage.hpp"
#include "Game/System/GameDataPlayerStatus.hpp"
#include "Game/System/GameDataTicoFatStorage.hpp"
#include "Game/System/GameDataWorldMapStorage.hpp"
#include "Game/System/GameEventFlagStorage.hpp"
#include "Game/System/GameEventValueChecker.hpp"
#include "Game/Util/MathUtil.hpp"
#include <cstdio>

GameDataHolder::GameDataHolder()
    : mEventFlagStorage(), mEventValueChecker(), mPlayerStatus(), mAllGalaxyStorage(), mTicoFatStorage(), mWorldMapStorage(), mChunkHolder() {
    mAllGalaxyStorage = new GameDataAllGalaxyStorage();
    mEventFlagStorage = new GameEventFlagStorage();
    mEventValueChecker = new GameEventValueChecker();
    mPlayerStatus = new GameDataPlayerStatus();
    mTicoFatStorage = new GameDataTicoFatStorage();
    mWorldMapStorage = new GameDataWorldMapStorage();

    mChunkHolder = new BinaryDataChunkHolder(4096, 6);
    mChunkHolder->addChunk(mPlayerStatus);
    mChunkHolder->addChunk(mEventFlagStorage);
    mChunkHolder->addChunk(mTicoFatStorage);
    mChunkHolder->addChunk(mEventValueChecker);
    mChunkHolder->addChunk(mAllGalaxyStorage);
    mChunkHolder->addChunk(mWorldMapStorage);

    resetAllData();

    snprintf(mName, sizeof(mName), "user1");
}

bool GameDataHolder::isOnGameEventFlag(const char* pFlagName) const {
    return mEventFlagStorage->isOn(pFlagName);
}

void GameDataHolder::onGameEventFlag(const char* pFlagName) {
    mEventFlagStorage->set(pFlagName, true);
}

void GameDataHolder::offGameEventFlag(const char* pFlagName) {
    mEventFlagStorage->set(pFlagName, false);
}

u16 GameDataHolder::getGameEventValue(const char* pName) const {
    return mEventValueChecker->getValue(pName);
}

void GameDataHolder::setGameEventValue(const char* pName, u16 value) {
    mEventValueChecker->setValue(pName, value);
}

void GameDataHolder::incPlayerMissNum() {
    u16 missNum = mEventValueChecker->getValue("累積死亡回数");
    s32 clamped = MR::clamp(static_cast< s32 >(missNum), 0, 9999);
    s32 newValue = MR::clamp(clamped + 1, 0, 9999);

    setGameEventValue("累積死亡回数", newValue);
}

s32 GameDataHolder::getPlayerMissNum() const {
    return MR::clamp(static_cast< s32 >(static_cast< u16 >(mEventValueChecker->getValue("累積死亡回数"))), 0, 9999);
}

s32 GameDataHolder::calcCurrentPowerStarNum() const {
    return mAllGalaxyStorage->calcCurrentPowerStarNum();
}

s32 GameDataHolder::calcNormalPowerStarNum() const {
    return mAllGalaxyStorage->calcNormalPowerStarNum();
}

bool GameDataHolder::hasGrandStar(int index) const {
    GameDataSomeScenarioAccessor* pScenario = mAllGalaxyStorage->makeAccessor(GameDataFunction::getWorldFinalGalaxyName(index))->getScenarioAccessor(1);
    return static_cast< u8 >(pScenario->mFlag & 1) == 1;
}

s32 GameDataHolder::calcTicoCoinNum() const {
    return mAllGalaxyStorage->calcTicoCoinNum();
}

s32 GameDataHolder::getStocketStarPieceNum() const {
    return mPlayerStatus->getStockedStarPieceNum();
}

void GameDataHolder::addStockedStarPieceNum(int num) {
    mPlayerStatus->addStockedStarPieceNum(num);
}

void GameDataHolder::incCumulativePlayTime() {
    s32 maxPlayTime = 0x0CDFD7F0;
    s32 playTime = MR::clamp(mEventValueChecker->getU32Value("累積プレイ時間"), 0, maxPlayTime);
    s32 newPlayTime = MR::clamp(playTime + 1, 0, maxPlayTime);
    mEventValueChecker->setU32Value("累積プレイ時間", newPlayTime);
}

s32 GameDataHolder::getCumulativePlayTime() const {
    s32 maxPlayTime = 0x0CDFD7F0;
    return MR::clamp(mEventValueChecker->getU32Value("累積プレイ時間"), 0, maxPlayTime);
}

void GameDataHolder::resetAllData() {
    mAllGalaxyStorage->initializeData();
    mEventFlagStorage->initializeData();
    mEventValueChecker->initializeData();
    mPlayerStatus->initializeData();
    mTicoFatStorage->initializeData();
    mWorldMapStorage->initializeData();
}

u32 GameDataHolder::makeFileBinary(u8* pData, u32 a2) {
    return mChunkHolder->makeFileBinary(pData, a2);
}

bool GameDataHolder::loadFromFileBinary(const char* pName, const u8* pData, u32 dataSize) {
    snprintf(mName, sizeof(mName), "%s", pName);
    return mChunkHolder->loadFromFileBinary(pData, dataSize);
}
