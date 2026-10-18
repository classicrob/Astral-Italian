#pragma once

#include <revolution/types.h>

struct AudSeLifeEntry {
    AudSeLifeEntry();  // fn_80085CF0

    /* 0x00 */ s32 mFrame;
    /* 0x04 */ s32 mLimit;
    /* 0x08 */ bool mIsActive;
    /* 0x09 */ bool mIsUsed;
};

// 128 timed SE slots (frame counter with a limit); slot allocation is fn_80085DE0, per-frame update fn_80085E50.
class AudSeLifeTable {
public:
    AudSeLifeTable();  // fn_80085D60
    void update();     // fn_80085E50

    /* 0x00 */ AudSeLifeEntry* mEntries;
};
