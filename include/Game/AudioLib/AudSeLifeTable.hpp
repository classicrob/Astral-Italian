#pragma once

#include <revolution/types.h>

struct AudSeLifeEntry {
    AudSeLifeEntry() {}
    void init();            // 0x80085CF0
    void start(s32 limit);  // 0x80085D10
    void add(s32 frames);   // 0x80085D30, mLimit += frames
    void reset();           // 0x80085D40

    /* 0x00 */ s32 mFrame;
    /* 0x04 */ s32 mLimit;
    /* 0x08 */ bool mIsActive;
    /* 0x09 */ bool mIsUsed;
};

// 128 timed SE slots (frame counter with a limit); slot allocation is fn_80085DE0, per-frame update fn_80085E50.
class AudSeLifeTable {
public:
    AudSeLifeTable();  // fn_80085D60
    AudSeLifeEntry* allocEntry(s32 limit);  // fn_80085DE0: first unused slot (slot 0 if full)
    void update();     // fn_80085E50

    /* 0x00 */ AudSeLifeEntry* mEntries;
};
