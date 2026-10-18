#include "Game/System/GameDataTicoFatStorage.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>

// PLACEHOLDER(core): fn_804DE1B0 / fn_804DE210 / fn_804DE280 are the feed-name hash list helpers of
// MR::Vector<MR::AssignableArray<u16>> in the unowned auto_03 gap (ctor(capacity) with zeroed count,
// contains(name), push(hash, name) which ignores its name when it is null); resolve when named.
extern "C" {
    MR::Vector< MR::AssignableArray< u16 > >* fn_804DE1B0(MR::Vector< MR::AssignableArray< u16 > >*, s32);
    bool fn_804DE210(const MR::Vector< MR::AssignableArray< u16 > >*, const char*);
    void fn_804DE280(MR::Vector< MR::AssignableArray< u16 > >*, u16, const char*);
}

GameDataTicoFatStorage::GameDataTicoFatStorage() : mStarPieceAlms() {
    mStarPieceAlms = new u16[48];
    MR::Vector< MR::AssignableArray< u16 > >* pList = (MR::Vector< MR::AssignableArray< u16 > >*)::operator new(sizeof(MR::Vector< MR::AssignableArray< u16 > >));
    if (pList != nullptr) {
        pList = fn_804DE1B0(pList, 16);
    }
    mTicoCoinFeedList = pList;
    initializeData();
}

s32 GameDataTicoFatStorage::getValue(int idx) const {
    return mStarPieceAlms[idx];
}

// NONMATCHING: target reloads mStarPieceAlms after the clamp and keeps the byte index in r7 (register allocation/CSE only)
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
        u16 value = 0;
        if (i < mTicoCoinFeedList->size()) {
            value = (*mTicoCoinFeedList)[i];
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
            fn_804DE280(mTicoCoinFeedList, value, nullptr);
        }
    }

    return 0;
}

void GameDataTicoFatStorage::initializeData() {
    MR::zeroMemory(mStarPieceAlms, 0x60);
    mTicoCoinFeedList->clear();
}

void GameDataTicoFatStorage::onTicoCoinFeed(const char* pName) {
    MR::Vector< MR::AssignableArray< u16 > >* pList = mTicoCoinFeedList;
    fn_804DE280(pList, (u16)MR::getHashCode(pName), pName);
}

bool GameDataTicoFatStorage::isTicoCoinFeed(const char* pName) const {
    return fn_804DE210(mTicoCoinFeedList, pName);
}
