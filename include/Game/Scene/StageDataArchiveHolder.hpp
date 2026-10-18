#pragma once

#include "Game/Util/Array.hpp"

// Per-directory stage archive object returned by StageDataArchiveHolder::mountArchive (0xE8 bytes).
// Only the three name lists used by the StageDataArchiveHolder constructor are laid out so far.
class StageDataArchive {
public:
    // PLACEHOLDER(core): 0x00 and 0xD0..0xE8 are not laid out yet; resolve when StageDataArchive is ported.
    /* 0x00 */ u32 _00;
    /* 0x04 */ MR::Vector< MR::FixedArray< const char*, 16 > > mNameList0;
    /* 0x48 */ MR::Vector< MR::FixedArray< const char*, 16 > > mNameList1;
    /* 0x8C */ MR::Vector< MR::FixedArray< const char*, 16 > > mNameList2;
};

// SMG2 stage archive holder. Mounts the Map / Sound / Design / ZoneInfo / UseResource / Ghost archives of one stage.
class StageDataArchiveHolder {
public:
    StageDataArchiveHolder(const char*);

    StageDataArchive* mountArchive(const char*, const char*);   // SMG2 0x8045B530 (defined with StageDataHolder)
    void addToArchives(void*, const char*);                    // SMG2 fn_8045B280 (provisional name)

    /* 0x00 */ StageDataArchive* mMapArchive;
    /* 0x04 */ StageDataArchive* mSoundArchive;
    /* 0x08 */ StageDataArchive* mDesignArchive;
    /* 0x0C */ StageDataArchive* mZoneInfoArchive;
    /* 0x10 */ StageDataArchive* mUseResourceArchive;
    /* 0x14 */ StageDataArchive* mGhostArchive;
};
