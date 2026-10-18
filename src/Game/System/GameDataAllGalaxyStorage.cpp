#include "Game/System/GameDataGalaxyStorage.hpp"
#include "Game/System/BinaryDataContentAccessor.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/StringUtil.hpp"

GameDataAllGalaxyStorage::GameDataAllGalaxyStorage() : mHeaderSerializer(nullptr), mScenarioHeaderSerializer(nullptr) {
    mHeaderSerializer = new BinaryDataContentHeaderSerializer(new u8[64], 64);
    mScenarioHeaderSerializer = new BinaryDataContentHeaderSerializer(new u8[64], 64);
    mSomeGalaxyStorages.init(96);

    ScenarioDataIter iter = MR::makeBeginScenarioDataIter();
    while (!iter.isEnd()) {
        GalaxyStatusAccessor accessor = iter.makeAccessor();
        if (accessor.getPowerStarNum() != 0) {
            mSomeGalaxyStorages.push_back(new GameDataSomeGalaxyStorage(accessor));
        }
        iter.goNext();
    }
    initHeaderSerializer();
}

GameDataSomeGalaxyStorage* GameDataAllGalaxyStorage::makeAccessor(const char* pGalaxyName) const {
    return mSomeGalaxyStorages[findIndex(pGalaxyName)];
}

s32 GameDataAllGalaxyStorage::calcCurrentPowerStarNum() const {
    s32 powerStarNum = 0;
    for (s32 idx = 0; idx < getGalaxyNum(); idx++) {
        powerStarNum += mSomeGalaxyStorages[idx]->getPowerStarNumOwned();
    }
    return powerStarNum;
}

s32 GameDataAllGalaxyStorage::calcNormalPowerStarNum() const {
    s32 normalPowerStarNum = 0;
    for (s32 idx = 0; idx < getGalaxyNum(); idx++) {
        GameDataSomeGalaxyStorage* pStorage = mSomeGalaxyStorages[idx];
        GalaxyStatusAccessor accessor = MR::makeGalaxyStatusAccessor(pStorage->mGalaxyName);
        for (s32 scenarioNo = 1; scenarioNo <= pStorage->getPowerStarNumOwned(); scenarioNo++) {
            GameDataSomeScenarioAccessor* pScenario = &pStorage->mScenarioAccessors[scenarioNo - 1];
            if (!pScenario->testFlag(1)) {
                continue;
            }
            if (pScenario->testFlag(2)) {
                continue;
            }
            if (accessor.isGreenStar(scenarioNo)) {
                continue;
            }
            normalPowerStarNum++;
        }
    }
    return normalPowerStarNum;
}

s32 GameDataAllGalaxyStorage::getPowerStarNumOwned(const char* pGalaxyName) const {
    return mSomeGalaxyStorages[findIndex(pGalaxyName)]->getPowerStarNumOwned();
}

s32 GameDataAllGalaxyStorage::calcTicoCoinNum() const {
    s32 ticoCoinNum = 0;
    for (s32 idx = 0; idx < getGalaxyNum(); idx++) {
        if (((u32)mSomeGalaxyStorages[idx]->mFlag & 1) == 1) {
            ticoCoinNum++;
        }
    }
    return ticoCoinNum;
}

u32 GameDataAllGalaxyStorage::makeHeaderHashCode() const {
    s32 scenarioDataSize = mScenarioHeaderSerializer->getDataSize();
    s32 galaxyHeaderSize = mHeaderSerializer->getHeaderSize();
    return galaxyHeaderSize + scenarioDataSize + 2;
}

u32 GameDataAllGalaxyStorage::getSignature() const {
    return 'GALA';
}

s32 GameDataAllGalaxyStorage::serialize(u8* pData, u32 dataSize) const {
    u16 galaxyNum = getGalaxyNum();
    *(u16*)pData = galaxyNum;

    MR::copyMemory(pData + 2, mHeaderSerializer->getBuffer(), mHeaderSerializer->getHeaderSize());
    s32 writeOffset = mHeaderSerializer->getHeaderSize() + 2;

    MR::copyMemory(pData + writeOffset, mScenarioHeaderSerializer->getBuffer(), mScenarioHeaderSerializer->getHeaderSize());
    writeOffset += mScenarioHeaderSerializer->getHeaderSize();

    BinaryDataContentAccessor accessor((u8*)mHeaderSerializer->getBuffer());
    BinaryDataContentAccessor scenarioAccessor((u8*)mScenarioHeaderSerializer->getBuffer());

    s32 loopCount = getGalaxyNum();
    for (s32 idx = 0; idx < loopCount; idx++) {
        GameDataSomeGalaxyStorage* pStorage = mSomeGalaxyStorages[idx];
        u8* pRecord = pData + writeOffset;

        u16 galaxyNameHash = MR::getHashCode(pStorage->mGalaxyName);
        u16* pGalaxyName = (u16*)accessor.getPointer("mGalaxyName", pRecord);
        *pGalaxyName = galaxyNameHash;

        u16 recordSize = pStorage->serialize(accessor, scenarioAccessor, pRecord);

        u16* pDataSize = (u16*)accessor.getPointer("mDataSize", pRecord);
        *pDataSize = recordSize;

        writeOffset += recordSize;
    }

    return writeOffset;
}

s32 GameDataAllGalaxyStorage::deserialize(const u8* pData, u32 dataSize) {
    u16 galaxyNum = *(u16*)pData;
    BinaryDataContentAccessor accessor((u8*)(pData + 2));
    s32 readOffset = accessor.getHeaderSize() + 2;

    BinaryDataContentAccessor scenarioAccessor((u8*)(pData + readOffset));
    readOffset += scenarioAccessor.getHeaderSize();

    bool readError = false;
    for (s32 idx = 0; idx < galaxyNum; idx++) {
        const u8* pRecord = pData + readOffset;

        u16* pGalaxyName = (u16*)accessor.getPointer("mGalaxyName", (u8*)pRecord);
        u16 galaxyNameHash = (pGalaxyName != nullptr) ? *pGalaxyName : 0;
        if (galaxyNameHash == 0) {
            return 2;
        }

        u16* pDataSize = (u16*)accessor.getPointer("mDataSize", (u8*)pRecord);
        u16 recordSize = (pDataSize != nullptr) ? *pDataSize : 0;
        if (recordSize == 0) {
            return 2;
        }

        s32 galaxyIndex = findIndex(galaxyNameHash);
        if (galaxyIndex >= 0) {
            u32 result = mSomeGalaxyStorages[galaxyIndex]->deserialize(accessor, scenarioAccessor, pRecord);
            if (result == 1) {
                readError = true;
            }
        } else {
            readError = true;
        }

        readOffset += recordSize;
    }

    return readError;
}

void GameDataAllGalaxyStorage::initializeData() {
    for (s32 idx = 0; idx < getGalaxyNum(); idx++) {
        mSomeGalaxyStorages[idx]->resetAllData();
    }
}

void GameDataAllGalaxyStorage::initHeaderSerializer() {
    mHeaderSerializer->addAttribute("mGalaxyName", sizeof(u16));
    mHeaderSerializer->addAttribute("mDataSize", sizeof(u16));
    mHeaderSerializer->addAttribute("mScenarioNum", sizeof(u8));
    mHeaderSerializer->addAttribute("mGalaxyState", sizeof(u8));
    mHeaderSerializer->addAttribute("mFlag", sizeof(u8));
    mHeaderSerializer->flush();

    mScenarioHeaderSerializer->addAttribute("mMissNum", sizeof(u8));
    mScenarioHeaderSerializer->addAttribute("mBestTime", sizeof(u32));
    mScenarioHeaderSerializer->addAttribute("mFlag", sizeof(u8));
    mScenarioHeaderSerializer->flush();
}

s32 GameDataAllGalaxyStorage::findIndex(const char* pGalaxyName) const {
    for (s32 idx = 0; idx < getGalaxyNum(); idx++) {
        if (MR::isEqualString(pGalaxyName, mSomeGalaxyStorages[idx]->mGalaxyName)) {
            return idx;
        }
    }
    return -1;
}

s32 GameDataAllGalaxyStorage::findIndex(u16 galaxyNameHash) const {
    for (s32 idx = 0; idx < getGalaxyNum(); idx++) {
        if ((u16)MR::getHashCode(mSomeGalaxyStorages[idx]->mGalaxyName) == galaxyNameHash) {
            return idx;
        }
    }
    return -1;
}
