#pragma once

// TODO

#include <revolution.h>

#define NUM_RHYTHM_INFO 6

class AudRhythmSeqParser;

// TODO: replace with proper struct/class
struct RhythmInfo {
    /* 0x00 */ bool mIsActive;
    /* 0x04 */ u32 _4;
    /* 0x08 */ u32 _8;
    /* 0x0C */ f32 _C;
    /* 0x10 */ s64 mStartTime;
    /* 0x18 */ s64 mPlayTime;
    /* 0x20 */ s64 mLastMeasuredTime;
    /* 0x28 */ s64 mPausedTime;
    /* 0x30 */ u32 _30;
    /* 0x34 */ u32 _34;
    /* 0x38 */ u8 _38[0x8];
};  // 0x40 bytes per record (the holder's mInfos stride)

// SMG2 unnamed helpers in the AudRhythmHolder split: index a 0x40-byte track record table at pBase (named by address).
f32 fn_8042A170(s32 pBase, s32 index);
f32 fn_8042A1D0(s32 pBase, s32 index);
f32 fn_8042A230(s32 pBase, s32 index);
f32 fn_8042A280(s32 pBase, s32 index);

class AudRhythmHolder {
public:
    AudRhythmHolder();

    ~AudRhythmHolder(){};

    void movement();
    void copyInfoFromParser();
    void init();
    void copyInfoFromParserOne(u32, u32, u32, s64);

    /* 0x000 */ f32 mModifiedTempo;
    /* 0x004 */ f32 mTempo;
    /* 0x008 */ f32 _8;
    /* 0x00C */ bool mIsPaused;
    /* 0x010 */ RhythmInfo mInfos[6];
    /* 0x190 */ bool mIsRhythmActive;
    /* 0x194 */ AudRhythmSeqParser* mSeqParser;
    /* 0x198 */ u32 _168;
    /* 0x19C */ u32 _16C;
};
