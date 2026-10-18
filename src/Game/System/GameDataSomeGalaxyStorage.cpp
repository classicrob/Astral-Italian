#include "Game/System/GameDataGalaxyStorage.hpp"
#include "Game/System/BinaryDataContentAccessor.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"

GameDataSomeGalaxyStorage::GameDataSomeGalaxyStorage(const GalaxyStatusAccessor& rAccessor) {
    mGalaxyName = rAccessor.getName();
    u8 scenarioNum = rAccessor.getPowerStarNum();
    mScenarioNum = scenarioNum;
    mGalaxyState = 0;
    mFlag = 0;
    mScenarioAccessors = nullptr;
    mScenarioAccessorNum = 0;
    mScenarioAccessors = new GameDataSomeScenarioAccessor[scenarioNum];
    mScenarioAccessorNum = scenarioNum;
    resetAllData();
}

void GameDataSomeGalaxyStorage::resetAllData() {
    mGalaxyState = 0;
    mFlag = 0;
    for (s32 idx = 0; idx < mScenarioAccessorNum; idx++) {
        mScenarioAccessors[idx].resetAllData();
    }
}

GameDataSomeScenarioAccessor* GameDataSomeGalaxyStorage::getScenarioAccessor(s32 scenarioNo) const {
    return &mScenarioAccessors[scenarioNo - 1];
}

s32 GameDataSomeGalaxyStorage::getPowerStarNumOwned() const {
    s32 numOwned = 0;
    for (s32 idx = 0; idx < mScenarioAccessorNum; idx++) {
        if (mScenarioAccessors[idx].testFlag(1)) {
            numOwned++;
        }
    }
    return numOwned;
}

// NONMATCHING: name strings come from the 0x40-byte table lbl_80700660 shared with the unsplit GameDataSomeScenarioAccessor code (0x804DCD30..0x804DCFE0), our literal pool has different offsets; same instructions otherwise
s32 GameDataSomeGalaxyStorage::serialize(const BinaryDataContentAccessor& rAccessor, const BinaryDataContentAccessor& rScenarioAccessor, u8* pData) const {
    u8 scenarioNum = mScenarioNum;
    *(u8*)rAccessor.getPointer("mScenarioNum", pData) = scenarioNum;

    u8 galaxyState = mGalaxyState;
    *(u8*)rAccessor.getPointer("mGalaxyState", pData) = galaxyState;

    u8 flag = mFlag;
    *(u8*)rAccessor.getPointer("mFlag", pData) = flag;

    s32 writeOffset = rAccessor.getDataSize();
    for (s32 idx = 0; idx < mScenarioAccessorNum; idx++) {
        mScenarioAccessors[idx].serialize(rScenarioAccessor, pData + writeOffset);
        writeOffset += rScenarioAccessor.getDataSize();
    }

    return writeOffset;
}

// NONMATCHING: same shared-literal-table offset difference as serialize
u32 GameDataSomeGalaxyStorage::deserialize(const BinaryDataContentAccessor& rAccessor, const BinaryDataContentAccessor& rScenarioAccessor, const u8* pData) {
    u8* scenarioNumPtr = (u8*)rAccessor.getPointer("mScenarioNum", (u8*)pData);
    u8 scenarioNum = (scenarioNumPtr != nullptr) ? *scenarioNumPtr : 0;
    if (scenarioNum == 0) {
        return 2;
    }

    u8* galaxyStatePtr = (u8*)rAccessor.getPointer("mGalaxyState", (u8*)pData);
    mGalaxyState = (galaxyStatePtr != nullptr) ? *galaxyStatePtr : 0;

    u8* flagPtr = (u8*)rAccessor.getPointer("mFlag", (u8*)pData);
    mFlag = (flagPtr != nullptr) ? *flagPtr : 0;

    s32 readOffset = rAccessor.getDataSize();
    for (s32 idx = 0; idx < ((mScenarioAccessorNum < scenarioNum) ? mScenarioAccessorNum : scenarioNum); idx++) {
        u32 result = mScenarioAccessors[idx].deserialize(rScenarioAccessor, pData + readOffset);
        if (result != 0) {
            return result;
        }
        readOffset += rScenarioAccessor.getDataSize();
    }

    return 0;
}
