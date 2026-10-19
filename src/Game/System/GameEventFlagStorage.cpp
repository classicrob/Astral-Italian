#include "Game/System/GameEventFlagStorage.hpp"
#include "Game/System/GameEventFlag.hpp"
#include "Game/System/GameEventFlagTable.hpp"
#include "Game/Util/BitArray.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>

GameEventFlagStorage::GameEventFlagStorage() : mUnk04(nullptr), mFlagBitArray(nullptr) {
    SingletonHolder< GameEventFlagTableInstance >::init();
    mFlagBitArray = new MR::BitArray(GameEventFlagTable::getTableSize());
}

void GameEventFlagStorage::set(const char* pFlagName, bool isOn) {
    mFlagBitArray->set(GameEventFlagTable::getIndex(GameEventFlagTable::findFlag(pFlagName)), isOn);
}

bool GameEventFlagStorage::isOn(const char* pFlagName) const {
    return mFlagBitArray->isOn(GameEventFlagTable::getIndex(GameEventFlagTable::findFlag(pFlagName)));
}

u32 GameEventFlagStorage::getSignature() const {
    return 'FLG1';
}

s32 GameEventFlagStorage::serialize(u8* pData, u32 size) const {
    JSUMemoryOutputStream stream(pData, size);

    for (GameEventFlagIter iter = GameEventFlagTable::getBeginIter(); !iter.isEnd(); iter.goNext()) {
        GameEventFlagAccessor accessor(iter.getFlag());
        u16 hash = MR::getHashCode(accessor.getName()) & 0x7FFF;
        u16 data = hash | (mFlagBitArray->isOn(GameEventFlagTable::getIndex(iter.getFlag())) ? 0x8000 : 0);
        stream.write(&data, sizeof(data));
    }

    return stream.mPosition;
}

s32 GameEventFlagStorage::deserialize(const u8* pData, u32 size) {
    bool isOn;
    s32 deserializationFailed = false;
    JSUMemoryInputStream stream(pData, size);

    int num = (int)size / (int)sizeof(u16);

    for (s32 i = 0; i < num; i++) {
        u16 data;
        stream.read(&data, sizeof(data));

        isOn = ((data >> 15) & 0x1) != 0;
        int index = GameEventFlagTable::getIndexFromHashCode(data & 0x7FFF);

        if (index >= 0) {
            mFlagBitArray->set(index, isOn);
        } else {
            deserializationFailed = true;
        }
    }

    return deserializationFailed ? true : false;
}

void GameEventFlagStorage::initializeData() {
    for (int i = 0; i < mFlagBitArray->size(); i++) {
        mFlagBitArray->set(i, false);
    }
}

u32 GameEventFlagStorage::makeHeaderHashCode() const {
    return MR::getHashCode("2bytes/flag");
}
