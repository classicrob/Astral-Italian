#include "Game/System/GameEventValueChecker.hpp"
#include "Game/System/FindingLuigiEventScheduler.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <cstdio>
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>

namespace {
    char sGliderJungleHiKey[] = "グライダー[ジャングル]/hi";
    char sGliderJungleLoKey[] = "グライダー[ジャングル]/lo";
    char sGliderChallengeHiKey[] = "グライダー[チャレンジ]/hi";
    char sGliderChallengeLoKey[] = "グライダー[チャレンジ]/lo";
    char sBestScoreMokumokuValleyLoKey[] = "ベストスコア[MokumokuValleyGalaxy]/lo";
    char sBestScoreMokumokuValleyHiKey[] = "ベストスコア[MokumokuValleyGalaxy]/hi";
    char sBestScoreHoneyBeeVillageLoKey[] = "ベストスコア[HoneyBeeVillageGalaxy]/lo";
    char sBestScoreHoneyBeeVillageHiKey[] = "ベストスコア[HoneyBeeVillageGalaxy]/hi";
    char sBestScoreUnderGroundDangeonLoKey[] = "ベストスコア[UnderGroundDangeonGalaxy]/lo";
    char sBestScoreUnderGroundDangeonHiKey[] = "ベストスコア[UnderGroundDangeonGalaxy]/hi";
    char sBestScoreTwisterTowerLoKey[] = "ベストスコア[TwisterTowerGalaxy]/lo";
    char sBestScoreTwisterTowerHiKey[] = "ベストスコア[TwisterTowerGalaxy]/hi";
    char sBestScoreKachikochiLavaLoKey[] = "ベストスコア[KachikochiLavaGalaxy]/lo";
    char sBestScoreKachikochiLavaHiKey[] = "ベストスコア[KachikochiLavaGalaxy]/hi";
    char sBestScoreWhiteSnowLoKey[] = "ベストスコア[WhiteSnowGalaxy]/lo";
    char sBestScoreWhiteSnowHiKey[] = "ベストスコア[WhiteSnowGalaxy]/hi";
    char sPostmanTaskRead0Key[] = "郵便屋[タスク手紙既読フラグ]/0";
    char sPostmanTaskRead1Key[] = "郵便屋[タスク手紙既読フラグ]/1";
    char sPostmanImportantRead0Key[] = "郵便屋[重要手紙既読フラグ]/0";
    char sPostmanImportantRead1Key[] = "郵便屋[重要手紙既読フラグ]/1";
    char sPostmanLastTaskIndexKey[] = "郵便屋[最後に読んだタスク手紙インデックス]";
    char sPostmanPeachLetterDeathCountKey[] = "郵便屋[ピーチ手紙を読んだ時の累積死亡回数]";
    char sMessageRead0Key[] = "メッセージ既読フラグ/0";
    char sMessageRead1Key[] = "メッセージ既読フラグ/1";
    char sDeathCountKey[] = "累積死亡回数";
    char sGameOverCountKey[] = "累積ゲームオーバー回数";
    char sPlayTimeLoKey[] = "累積プレイ時間/lo";
    char sPlayTimeHiKey[] = "累積プレイ時間/hi";
    char sLuigiAppearCountKey[] = "でしゃばりルイージ出現カウンタ";
    char sKinopioBankInterestKey[] = "銀行屋キノピオ[利子]";
    char sFaceShipEvent0Key[] = "顔惑星イベント番号/0";
    char sFaceShipEvent1Key[] = "顔惑星イベント番号/1";
    char sFaceShipEvent2Key[] = "顔惑星イベント番号/2";
    char sFaceShipEvent3Key[] = "顔惑星イベント番号/3";
    char sFaceShipGrandStarEventKey[] = "顔惑星イベントグランドスター番号";
    char sStageClearAfterDeathsCountKey[] = "一定数死亡後のステージクリア回数";

    const GameEventValue cGameEventValueTable[] = {
        {sGliderJungleHiKey, 0},
        {sGliderJungleLoKey, 90 * 60},
        {sGliderChallengeHiKey, 0},
        {sGliderChallengeLoKey, 90 * 60},
        {sBestScoreMokumokuValleyLoKey, 0},
        {sBestScoreMokumokuValleyHiKey, 0},
        {sBestScoreHoneyBeeVillageLoKey, 0},
        {sBestScoreHoneyBeeVillageHiKey, 0},
        {sBestScoreUnderGroundDangeonLoKey, 0},
        {sBestScoreUnderGroundDangeonHiKey, 0},
        {sBestScoreTwisterTowerLoKey, 0},
        {sBestScoreTwisterTowerHiKey, 0},
        {sBestScoreKachikochiLavaLoKey, 0},
        {sBestScoreKachikochiLavaHiKey, 0},
        {sBestScoreWhiteSnowLoKey, 0},
        {sBestScoreWhiteSnowHiKey, 0},
        {sPostmanTaskRead0Key, 0},
        {sPostmanTaskRead1Key, 0},
        {sPostmanImportantRead0Key, 0},
        {sPostmanImportantRead1Key, 0},
        {sPostmanLastTaskIndexKey, 0},
        {sPostmanPeachLetterDeathCountKey, 0},
        {sMessageRead0Key, 0},
        {sMessageRead1Key, 0},
        {sDeathCountKey, 0},
        {sGameOverCountKey, 0},
        {sPlayTimeLoKey, 0},
        {sPlayTimeHiKey, 0},
        {sLuigiAppearCountKey, 0},
        {sKinopioBankInterestKey, 0},
        {sFaceShipEvent0Key, 0},
        {sFaceShipEvent1Key, 0},
        {sFaceShipEvent2Key, 0},
        {sFaceShipEvent3Key, 0},
        {sFaceShipGrandStarEventKey, 0},
        {sStageClearAfterDeathsCountKey, 0},
    };
};  // namespace

GameEventValueChecker::GameEventValueChecker() : mValues(), mNumValues() {
    mValues = new u16[ARRAY_SIZE(::cGameEventValueTable)];
    mNumValues = ARRAY_SIZE(::cGameEventValueTable);
    initializeData();
}

u16 GameEventValueChecker::getValue(const char* pName) const {
    u16* values;
    s32 index = findIndex(pName);
    values = mValues;
    return values[index];
}

void GameEventValueChecker::setValue(const char* pName, u16 value) {
    mValues[findIndex(pName)] = value;
}

u32 GameEventValueChecker::getU32Value(const char* pName) const {
    char hiName[128];
    snprintf(hiName, sizeof(hiName), "%s/hi", pName);
    u16 hi = getValue(hiName);

    char loName[128];
    snprintf(loName, sizeof(loName), "%s/lo", pName);
    u16 lo = getValue(loName);

    return hi << 16 | lo;
}

void GameEventValueChecker::setU32Value(const char* pName, u32 value) {
    char hiName[128];
    snprintf(hiName, sizeof(hiName), "%s/hi", pName);
    s32 hiIndex = findIndex(hiName);
    mValues[hiIndex] = value >> 16;

    char loName[128];
    snprintf(loName, sizeof(loName), "%s/lo", pName);
    s32 loIndex = findIndex(loName);
    mValues[loIndex] = value;
}

u16 GameEventValueChecker::getIndexedValue(const char* pName, s32 index) const {
    char name[128];
    snprintf(name, sizeof(name), "%s/%d", pName, index);
    return mValues[findIndex(name)];
}

void GameEventValueChecker::setIndexedValue(const char* pName, s32 index, u16 value) {
    char name[128];
    snprintf(name, sizeof(name), "%s/%d", pName, index);
    mValues[findIndex(name)] = value;
}

bool GameEventValueChecker::isOnIndexedBit(const char* pName, s32 bit) const {
    char name[128];
    snprintf(name, sizeof(name), "%s/%01d", pName, bit / 16);
    u16 value = mValues[findIndex(name)];
    return (value & 1 << (bit % 16)) != 0;
}

void GameEventValueChecker::onIndexedBit(const char* pName, s32 bit) {
    char name[128];
    snprintf(name, sizeof(name), "%s/%01d", pName, bit / 16);
    u16 value = getValue(name);
    u16 mask = 1 << (bit % 16);
    setValue(name, value | mask);
}

u32 GameEventValueChecker::makeHeaderHashCode() const {
    return getSignature();
}

u32 GameEventValueChecker::getSignature() const {
    return 'VLE1';
}

s32 GameEventValueChecker::serialize(u8* pData, u32 maxBufferSize) const {
    JSUMemoryOutputStream stream(pData, maxBufferSize);

    s32 hash;

    for (s32 idx = 0; idx < mNumValues; idx++) {
        hash = MR::getHashCode(::cGameEventValueTable[idx].mName);
        const u16 value = mValues[idx];

        stream.writeU16(hash);
        stream.writeU16(value);
    }

    return stream.mPosition;
}

s32 GameEventValueChecker::deserialize(const u8* pData, u32 maxBufferSize) {
    s32 readError = 0;

    JSUMemoryInputStream stream(pData, maxBufferSize);

    s32 numEntries = static_cast< s32 >(maxBufferSize) / 2;

    for (s32 idx = 0; idx < numEntries; idx++) {
        u16 hash = stream.readU16();
        u16 value = stream.readU16();

        s32 valueIndex = findIndexFromHashCode(hash);

        if (valueIndex >= 0) {
            mValues[valueIndex] = value;
        } else {
            readError = 1;
        }
    }

    if (stream.mState != JSUIosBase::IO_OK && stream.getState(JSUIosBase::IO_MEMORY_ERROR) == 0) {
        stream.mState = JSUIosBase::IO_OK;
    }

    return readError ? 1 : 0;
}

void GameEventValueChecker::initializeData() {
    for (s32 idx = 0; idx < mNumValues; idx++) {
        mValues[idx] = ::cGameEventValueTable[idx].mDefaultValue;
    }
}

s32 GameEventValueChecker::findIndex(const char* pName) const {
    for (s32 idx = 0; idx < mNumValues; idx++) {
        if (MR::isEqualString(pName, ::cGameEventValueTable[idx].mName)) {
            return idx;
        }
    }

    return -1;
}

s32 GameEventValueChecker::findIndexFromHashCode(u16 hash) const {
    for (s32 idx = 0; idx < mNumValues; idx++) {
        if (hash == static_cast< u16 >(MR::getHashCode(::cGameEventValueTable[idx].mName))) {
            return idx;
        }
    }

    return -1;
}
