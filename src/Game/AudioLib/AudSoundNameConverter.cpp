#include "Game/AudioLib/AudSoundNameConverter.hpp"
#include "Game/Util.hpp"
#include <JSystem/JAudio2/JAUSoundTable.hpp>
#include <JSystem/JGadget/hashcode.hpp>
#include <cstring>

namespace {
    static const char* sCategoryNamePrefix[] = {
        "SY",   "PV", "PM", "BV", "BM", "OJ", "AT", "DM", "EV", "EM", "SV", "SM", "RS",  //
        "HO",   "BG", "MB", "ST",                                                        //
        "\0\0",                                                                          //
    };

    // static const u8 sGroupNoTable[] = { };

    static bool isSeName(const char* pName) NO_INLINE;
    static bool isSeName(const char* pName) {
        if (pName[0] == 'S' && pName[1] == 'E') {
            return true;
        }
        return false;
    }
};  // namespace

inline s32 AudSoundNameConverter::getSeSoundCategory(const char* pName) const {
    for (s32 i = 0; i < 13; i++) {
        if (pName[3] == sCategoryNamePrefix[i][0] && pName[4] == sCategoryNamePrefix[i][1]) {
            return i;
        }
    }
    return -1;
}

inline s32 AudSoundNameConverter::getOtherSoundCategory(const char* pName) const {
    for (s32 i = 13; i < 17; i++) {
        if (pName[0] == sCategoryNamePrefix[i][0] && pName[1] == sCategoryNamePrefix[i][1]) {
            return i;
        }
    }
    return -1;
}

AudSoundNameConverter::AudSoundNameConverter()
    : mNumItems(), mNumNameDatas(), mGroupItemOffsets(), mNumItemsSection0(), mNumItemsSection1(), mNumItemsSection2(), mSoundNameData() {
    init();
}

JAISoundID AudSoundNameConverter::getSoundID(const char* pName) {
    return static_cast<const AudSoundNameConverter*>(this)->getSoundID(pName, JGadget::getHashCode(pName));
}

JAISoundID AudSoundNameConverter::getSoundID(const char* pName, u32 hash) {
    return static_cast<const AudSoundNameConverter*>(this)->getSoundID(pName, hash);
}

JAISoundID AudSoundNameConverter::getSoundID(const char* pName, u32 hash) const {
    s32 startingOffset;
    if (isSeName(pName)) {
        startingOffset = mGroupItemOffsets[getSeSoundCategory(pName)];
    } else {
        startingOffset = mGroupItemOffsets[getOtherSoundCategory(pName)];
    }

    for (s32 i = startingOffset; i < mNumItems; i++) {
        const AudSoundNameData& data = mSoundNameData[i];
        if (hash == data.mHash && strcmp(pName, data.mName) == 0) {
            return data.mID;
        }
    }
    return -1;
}

const char* AudSoundNameConverter::getSoundNameID(const JAISoundID& rID) const {
    return JAUSoundNameTable::getInstance()->getName(rID);
}

void AudSoundNameConverter::init() {
    const JAUSoundNameTable* table = JAUSoundNameTable::getInstance();
    initDataTable(table);
    addSectionInfo(table, 0);
    addSectionInfo(table, 1);
    addSectionInfo(table, 2);
}

void AudSoundNameConverter::initDataTable(const JAUSoundNameTable* pTable) {
    mGroupItemOffsets = new u32[17];
    mNumItems = 0;
    mNumItemsSection0 = getSoundNameTableNumItem_inSection(pTable, 0);
    mNumItemsSection1 = getSoundNameTableNumItem_inSection(pTable, 1);
    mNumItemsSection2 = getSoundNameTableNumItem_inSection(pTable, 2);

    mNumItems += mNumItemsSection0;
    mNumItems += mNumItemsSection1;
    mNumItems += mNumItemsSection2;

    mSoundNameData = new AudSoundNameData[mNumItems];

    for (s32 i = 0; i < mNumItems; i++) {
        mSoundNameData[i].mName = nullptr;
        mSoundNameData[i].mID.setAnonymous();
        mSoundNameData[i].mHash = 0;
    }
}

AudSoundNameData::AudSoundNameData() {
}

void AudSoundNameConverter::addGroupInfo(const JAUSoundNameTable* pTable, u8 sectionID, u8 groupID) {
    for (u16 i = 0; i < pTable->getNumItems_inGroup(sectionID, groupID); i++) {
        AudSoundNameData& data = mSoundNameData[mNumNameDatas];
        data.mID.set(sectionID, groupID, i);
        data.mName = pTable->getName(data.mID);
        data.mHash = JGadget::getHashCode(data.mName);
        mNumNameDatas++;
    }
}

void AudSoundNameConverter::addSectionInfo(const JAUSoundNameTable* pTable, u8 sectionID) {
    for (u8 i = 0; i < pTable->getNumGroups_inSection(sectionID); i++) {
        addGroupInfo(pTable, sectionID, i);
    }
}

u32 AudSoundNameConverter::getSoundNameTableNumItem_inSection(const JAUSoundNameTable* pTable, u8 sectionID) {
    u32 numItems = 0;
    for (u8 i = 0; i < pTable->getNumGroups_inSection(sectionID); i++) {
        switch (sectionID) {
        case 0:
            mGroupItemOffsets[i + 0] = numItems;
            break;
        case 1:
            mGroupItemOffsets[i + 14] = numItems + mNumItemsSection0;
            break;
        case 2:
            mGroupItemOffsets[i + 16] = numItems + mNumItemsSection0 + mNumItemsSection1;
            break;
        }

        numItems += pTable->getNumItems_inGroup(sectionID, i);
    }

    return numItems;
}

JAISoundID AudSoundNameConverter::getSoundIDFromName(const char* pName) {
    return get()->getSoundID(pName);
}
