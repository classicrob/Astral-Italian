#include "Game/AudioLib/AudSeLifeTable.hpp"

void AudSeLifeEntry::init() {
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
        mEntries[i].init();
    }
}

// NONMATCHING: CW keeps mEntries in r5 across the loop (return mEntries uses it); target recomputes with lwz r0 / add r5 and returns lwz r3,0(r3). Register allocation only.
AudSeLifeEntry* AudSeLifeTable::allocEntry(s32 limit) {
    for (s32 i = 0; i < 128; i++) {
        if (!mEntries[i].mIsUsed) {
            mEntries[i].start(limit);
            return &mEntries[i];
        }
    }
    return mEntries;
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
