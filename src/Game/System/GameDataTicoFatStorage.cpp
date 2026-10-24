#include "Game/System/GameDataTicoFatStorage.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>

TicoCoinFeedList::TicoCoinFeedList(s32 capacity) {
    init(capacity);
}

// NONMATCHING: regalloc (hash and iterator registers only) (FP order/grouping checked: no FP instructions).
bool TicoCoinFeedList::contains(const char* pName) const {
    const u16* pEnd;
    const u16* p;
    u32 hash = static_cast<u16>(MR::getHashCode(pName));
    p = begin();
    pEnd = end();
    while (p != pEnd && *p != hash) {
        ++p;
    }
    return p != pEnd;
}

void TicoCoinFeedList::add(u16 hash, const char*) {
    if (capacity() > size()) {
        push_back(hash);
    }
}

GameDataTicoFatStorage::GameDataTicoFatStorage() : mStarPieceAlms() {
    mStarPieceAlms = new u16[48];
    mTicoCoinFeedList = new TicoCoinFeedList(16);
    initializeData();
}

s32 GameDataTicoFatStorage::getValue(int idx) const {
    return mStarPieceAlms[idx];
}

// NONMATCHING: target reloads mStarPieceAlms after the clamp and keeps the byte index in r7 (register allocation/CSE only) (FP order/grouping checked: no FP instructions).
void GameDataTicoFatStorage::addValue(int idx, int add) {
    s32 value = mStarPieceAlms[idx] + add;
    mStarPieceAlms[idx] = MR::clamp(value, 0, 0x7FFF);
}

u32 GameDataTicoFatStorage::makeHeaderHashCode() const {
    return MR::getHashCode("SaveDataStorageTicoFat") + 0x120;
}

u32 GameDataTicoFatStorage::getSignature() const {
    return 'STF1';
}

s32 GameDataTicoFatStorage::serialize(u8* pData, u32 dataSize) const {
    JSUMemoryOutputStream stream(pData, dataSize);
    stream.write(mStarPieceAlms, 0x60);

    for (s32 i = 0; i < 16; i++) {
        u16 value;
        if (i < mTicoCoinFeedList->size()) {
            value = (*mTicoCoinFeedList)[i];
        } else {
            value = 0;
        }
        stream.write(&value, sizeof(u16));
    }

    return stream.getPosition();
}

s32 GameDataTicoFatStorage::deserialize(const u8* pData, u32 dataSize) {
    JSUMemoryInputStream stream(pData, dataSize);
    stream.read(mStarPieceAlms, 0x60);

    mTicoCoinFeedList->clear();
    for (s32 i = 0; i < 16; i++) {
        u16 value;
        stream.read(&value, sizeof(u16));
        if (value != 0) {
            mTicoCoinFeedList->add(value, nullptr);
        }
    }

    return 0;
}

void GameDataTicoFatStorage::initializeData() {
    MR::zeroMemory(mStarPieceAlms, 0x60);
    mTicoCoinFeedList->clear();
}

void GameDataTicoFatStorage::onTicoCoinFeed(const char* pName) {
    TicoCoinFeedList* pList = mTicoCoinFeedList;
    pList->add((u16)MR::getHashCode(pName), pName);
}

bool GameDataTicoFatStorage::isTicoCoinFeed(const char* pName) const {
    return mTicoCoinFeedList->contains(pName);
}
