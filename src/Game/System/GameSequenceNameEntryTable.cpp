#include "Game/System/GameSequence.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"

namespace {
    const char* sFlagNameTableName = "InStageFlagNameTable";
};  // namespace

GameSequenceNameEntry::GameSequenceNameEntry() : mValue(0), mZoneId(-1), mLinkId(-1) {
}

void GameSequenceNameEntry::set(const char* pName, s32 zoneId, s32 linkId) {
    mValue = MR::getHashCode(pName) & 0x7FFF;
    mZoneId = zoneId;
    mLinkId = linkId;
}

bool GameSequenceNameEntry::operator==(const GameSequenceNameEntry& rOther) const {
    return (u32)(rOther.mValue & 0x7FFF) == (u32)(mValue & 0x7FFF) && rOther.mZoneId == mZoneId && rOther.mLinkId == mLinkId;
}

#pragma dont_inline on
void GameSequenceNameEntry::onDone() {
    mValue = (mValue & 0x7FFF) | 0x8000;
}
#pragma dont_inline reset

GameSequenceNameEntryTable::GameSequenceNameEntryTable(s32 count) : mEntries(nullptr), mCapacity(0), mCount(0) {
    mEntries = new GameSequenceNameEntry[count];
    mCapacity = count;
}

void GameSequenceNameEntryTable::clear() {
    mCount = 0;
}

bool GameSequenceNameEntryTable::setupAlreadyDoneEntry(int* pOutIndex, const char* pName, const JMapInfoIter& rIter) {
    s32 linkId = -1;
    MR::getJMapInfoLinkID(rIter, &linkId);
    return registerEntry(pOutIndex, pName, MR::getPlacedZoneId(rIter), linkId);
}

void GameSequenceNameEntryTable::onAlreadyDoneEntry(const char* pName) {
    GameSequenceNameEntry* pEntry = find(pName);
    if (pEntry != nullptr) {
        pEntry->onDone();
    }
}

void GameSequenceNameEntryTable::updateAlreadyDoneEntry(s32 index) {
    GameSequenceNameEntry* pEntry = getEntry(index);
    if (pEntry != nullptr) {
        pEntry->onDone();
    }
}

bool GameSequenceNameEntryTable::isOnAlreadyDoneEntry(const char* pName) {
    GameSequenceNameEntry* pEntry = find(pName);
    if (pEntry == nullptr) {
        return false;
    }

    return pEntry->isDone();
}

void GameSequenceNameEntryTable::registerZoneNames() {
    const JMapInfo* pInfo = MR::tryCreateStageZoneInfoCsvParser(sFlagNameTableName);
    if (pInfo != nullptr) {
        s32 num = MR::getCsvDataElementNum(pInfo);
        const char* pName = nullptr;
        for (s32 i = 0; i < num; i++) {
            MR::getCsvDataStr(&pName, pInfo, "FlagName", i);
            registerEntry(nullptr, pName, -1, -1);
        }
    }
}

bool GameSequenceNameEntryTable::registerEntry(int* pOutIndex, const char* pName, s32 zoneId, s32 linkId) {
    GameSequenceNameEntry entry;
    entry.set(pName, zoneId, linkId);

    GameSequenceNameEntry* pEntry = mEntries;
    GameSequenceNameEntry* pEnd = &mEntries[mCount];
    while (pEntry != pEnd && !(*pEntry == entry)) {
        pEntry++;
    }

    if (pEntry != &mEntries[mCount]) {
        if (pOutIndex != nullptr) {
            *pOutIndex = pEntry - mEntries;
        }

        return pEntry->isDone();
    }

    if (pOutIndex != nullptr) {
        *pOutIndex = mCount;
    }

    mEntries[mCount++] = entry;
    return false;
}

GameSequenceNameEntry* GameSequenceNameEntryTable::find(const char* pName) {
    GameSequenceNameEntry entry;
    entry.set(pName, -1, -1);

    GameSequenceNameEntry* pEntry = mEntries;
    GameSequenceNameEntry* pEnd = &mEntries[mCount];
    while (pEntry != pEnd && !(*pEntry == entry)) {
        pEntry++;
    }

    if (pEntry == &mEntries[mCount]) {
        return nullptr;
    }

    return pEntry;
}

GameSequenceNameEntry* GameSequenceNameEntryTable::getEntry(s32 index) {
    if (index < 0 || mCount <= index) {
        return nullptr;
    }

    return &mEntries[index];
}
