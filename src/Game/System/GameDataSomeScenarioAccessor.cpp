#include "Game/System/GameDataGalaxyStorage.hpp"
#include "Game/System/BinaryDataContentAccessor.hpp"

GameDataSomeScenarioAccessor::GameDataSomeScenarioAccessor() {
    mFlag = 0;
    resetAllData();
}

void GameDataSomeScenarioAccessor::resetAllData() {
    mMissNum = 0;
    mBestTime = 0;
    mFlag = 0;
}

void GameDataSomeScenarioAccessor::serialize(const BinaryDataContentAccessor& rAccessor, u8* pData) const {
    *(u8*)rAccessor.getPointer("mMissNum", pData) = mMissNum;
    *(s32*)rAccessor.getPointer("mBestTime", pData) = mBestTime;
    *(u8*)rAccessor.getPointer("mFlag", pData) = mFlag;
}

s32 GameDataSomeScenarioAccessor::deserialize(const BinaryDataContentAccessor& rAccessor, const u8* pData) {
    u8 missNum;
    u8* pMissNum = (u8*)rAccessor.getPointer("mMissNum", (u8*)pData);
    if (pMissNum != NULL) {
        missNum = *pMissNum;
    } else {
        missNum = 0;
    }
    mMissNum = missNum;

    s32 bestTime;
    s32* pBestTime = (s32*)rAccessor.getPointer("mBestTime", (u8*)pData);
    if (pBestTime != NULL) {
        bestTime = *pBestTime;
    } else {
        bestTime = 0;
    }
    mBestTime = bestTime;

    u8 flag;
    u8* pFlag = (u8*)rAccessor.getPointer("mFlag", (u8*)pData);
    if (pFlag != NULL) {
        flag = *pFlag;
    } else {
        flag = 0;
    }
    mFlag = flag;
    return 0;
}
