#include "Game/AudioLib/AudSeLifeTable.hpp"

AudSeLifeEntry::AudSeLifeEntry() {
    mFrame = 0;
    mLimit = 0;
    mIsActive = false;
    mIsUsed = false;
}

void AudSeLifeEntry::start(s32 limit) {
    mFrame = 0;
    mLimit = limit;
    mIsActive = true;
    mIsUsed = true;
}

void AudSeLifeEntry::add(s32 frames) {
    mLimit += frames;
}

void AudSeLifeEntry::reset() {
    mFrame = 0;
    mLimit = 0;
    mIsActive = false;
    mIsUsed = false;
}

AudSeLifeTable::AudSeLifeTable() {
    mEntries = nullptr;
    mEntries = new AudSeLifeEntry[128];
    for (s32 i = 0; i < 128; i++) {
        mEntries[i].reset();
    }
}

AudSeLifeEntry* AudSeLifeTable::allocEntry(s32 limit) {
    for (s32 i = 0; i < 128; i++) {
        AudSeLifeEntry* pEntry = &mEntries[i];
        if (!pEntry->mIsUsed) {
            pEntry->start(limit);
            return &mEntries[i];
        }
    }
    return &mEntries[0];
}

void AudSeLifeTable::update() {
    for (s32 i = 0; i < 128; i++) {
        AudSeLifeEntry* pEntry = &mEntries[i];
        if (pEntry->mIsUsed == true && pEntry->mIsActive == true) {
            pEntry->mFrame++;
        }
        pEntry = &mEntries[i];
        if (pEntry->mIsUsed == true && pEntry->mFrame >= pEntry->mLimit) {
            pEntry->reset();
        }
    }
}
