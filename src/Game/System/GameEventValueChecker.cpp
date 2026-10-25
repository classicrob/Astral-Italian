#include "Game/System/GameEventValueChecker.hpp"
#include "Game/System/FindingLuigiEventScheduler.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <cstdio>
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>

namespace {
    const GameEventValue cGameEventValueTable[] = {
        {"グライダー[ジャングル]/hi", 0},
        {"グライダー[ジャングル]/lo", 90 * 60},
        {"グライダー[チャレンジ]/hi", 0},
        {"グライダー[チャレンジ]/lo", 90 * 60},
        {"ベストスコア[MokumokuValleyGalaxy]/lo", 0},
        {"ベストスコア[MokumokuValleyGalaxy]/hi", 0},
        {"ベストスコア[HoneyBeeVillageGalaxy]/lo", 0},
        {"ベストスコア[HoneyBeeVillageGalaxy]/hi", 0},
        {"ベストスコア[UnderGroundDangeonGalaxy]/lo", 0},
        {"ベストスコア[UnderGroundDangeonGalaxy]/hi", 0},
        {"ベストスコア[TwisterTowerGalaxy]/lo", 0},
        {"ベストスコア[TwisterTowerGalaxy]/hi", 0},
        {"ベストスコア[KachikochiLavaGalaxy]/lo", 0},
        {"ベストスコア[KachikochiLavaGalaxy]/hi", 0},
        {"ベストスコア[WhiteSnowGalaxy]/lo", 0},
        {"ベストスコア[WhiteSnowGalaxy]/hi", 0},
        {"郵便屋[タスク手紙既読フラグ]/0", 0},
        {"郵便屋[タスク手紙既読フラグ]/1", 0},
        {"郵便屋[重要手紙既読フラグ]/0", 0},
        {"郵便屋[重要手紙既読フラグ]/1", 0},
        {"郵便屋[最後に読んだタスク手紙インデックス]", 0},
        {"郵便屋[ピーチ手紙を読んだ時の累積死亡回数]", 0},
        {"メッセージ既読フラグ/0", 0},
        {"メッセージ既読フラグ/1", 0},
        {"累積死亡回数", 0},
        {"累積ゲームオーバー回数", 0},
        {"累積プレイ時間/lo", 0},
        {"累積プレイ時間/hi", 0},
        {"でしゃばりルイージ出現カウンタ", 0},
        {"銀行屋キノピオ[利子]", 0},
        {"顔惑星イベント番号/0", 0},
        {"顔惑星イベント番号/1", 0},
        {"顔惑星イベント番号/2", 0},
        {"顔惑星イベント番号/3", 0},
        {"顔惑星イベントグランドスター番号", 0},
        {"一定数死亡後のステージクリア回数", 0},
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
    setValue(hiName, value >> 16);

    char loName[128];
    snprintf(loName, sizeof(loName), "%s/lo", pName);
    setValue(loName, value);
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
