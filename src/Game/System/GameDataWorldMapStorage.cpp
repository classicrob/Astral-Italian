#include "Game/System/GameDataWorldMapStorage.hpp"
#include "Game/System/SysConfigFile.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>

GameDataWorldMapStorage::GameDataWorldMapStorage() {
    u8* pFlag = mPointFlag;
    u8* pEnd = &mCurrentWorldNo;
    do {
        *pFlag = 0;
        pFlag++;
    } while (pFlag < pEnd);
    initializeData();
}

u32 GameDataWorldMapStorage::makeHeaderHashCode() const {
    return MR::getHashCode("SaveDataStorageWorldMap") * 9;
}

u32 GameDataWorldMapStorage::getSignature() const {
    return 'SSWM';
}

s32 GameDataWorldMapStorage::serialize(u8* pData, u32 dataSize) const {
    JSUMemoryOutputStream stream(pData, dataSize);
    stream.write(mPointFlag, 8);
    u8 worldNo = mCurrentWorldNo;
    stream.write(&worldNo, 1);
    return stream.getPosition();
}

s32 GameDataWorldMapStorage::deserialize(const u8* pData, u32 dataSize) {
    JSUMemoryInputStream stream(pData, dataSize);
    stream.read(mPointFlag, 8);
    stream.read(&mCurrentWorldNo, 1);
    return 0;
}

void GameDataWorldMapStorage::initializeData() {
    MR::zeroMemory(mPointFlag, 8);
    mCurrentWorldNo = 1;
}

void GameDataWorldMapStorage::setCurrentWorldNo(u8 worldNo) {
    mCurrentWorldNo = worldNo;
}

bool GameDataWorldMapStorage::isOnPointFlag(s32 index, s32 bit) const {
    u8 mask = 1 << bit;
    return (mask & mPointFlag[index - 1]) == mask;
}

void GameDataWorldMapStorage::setPointFlag(s32 index, s32 bit, bool isOn) {
    u8& flag = mPointFlag[index - 1];
    if (isOn) {
        flag = flag | (u8)(1 << bit);
    } else {
        flag = flag & (u8)~(u8)(1 << bit);
    }
}

void SysConfigCounters::reset() {
    mBankedStarPiece = 0;
    mMaxBankedStarPiece = 0;
    mSharedPlayerLeft = 0;
    mPlayerLeftHash = 0;
}

void SysConfigCounters::copyFrom(const SysConfigCounters& other) {
    mBankedStarPiece = other.mBankedStarPiece;
    mMaxBankedStarPiece = other.mMaxBankedStarPiece;
    mSharedPlayerLeft = other.mSharedPlayerLeft;
    mPlayerLeftHash = other.mPlayerLeftHash;
}
