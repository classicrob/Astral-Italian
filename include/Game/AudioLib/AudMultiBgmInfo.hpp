#pragma once

#include <revolution/types.h>

struct AudMultiBgmEntry {
    u32 mID;
    u32 mSeqID;
    u32 mStreamID;
    f32 mBeatMul;
    u32 mIntroBeats;
    u32 mLoopBeats;
    u32 mLoopStartSamples;
    u32 mLoopEndSamples;
};

// MultiBgmInfo.bcsv table (/AudioRes/Info/MultiBgmInfo.arc)
class AudMultiBgmInfo {
public:
    AudMultiBgmInfo();  // fn_800838D0
    void load();        // fn_80083920

    AudMultiBgmEntry* findEntry(u32) NO_INLINE;
    u32 getSeqIdForMultiBgm(u32);
    u32 getStreamIdForMultiBgm(u32);
    f32 getBeatMulForMultiBgm(u32);
    u32 getIntroBeatsForMultiBgm(u32);
    u32 getLoopBeatsForMultiBgm(u32);
    u32 getLoopStartSamplesForMultiBgm(u32);
    u32 getLoopEndSamplesForMultiBgm(u32);

    /* 0x00 */ AudMultiBgmEntry* mEntries;  // 0xC80 bytes
    /* 0x04 */ s32 mNumEntries;
};
