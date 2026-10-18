#pragma once

#include <revolution/types.h>

// MultiBgmInfo.bcsv table (/AudioRes/Info/MultiBgmInfo.arc)
class AudMultiBgmInfo {
public:
    AudMultiBgmInfo();  // fn_800838D0
    void load();        // fn_80083920

    /* 0x00 */ void* mEntries;  // 0xC80 bytes
    /* 0x04 */ s32 mNumEntries;
};
