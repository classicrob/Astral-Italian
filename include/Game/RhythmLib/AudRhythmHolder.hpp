#pragma once

#include <revolution.h>

#define NUM_RHYTHM_INFO 6

class AudRhythmSeqParser;

struct RhythmInfo {
    /* 0x00 */ bool mIsActive;
    /* 0x04 */ u32 _4;
    /* 0x08 */ u32 _8;
    /* 0x0C */ f32 mNoteDurationSec;
    /* 0x10 */ s64 mStartTime;
    /* 0x18 */ s64 mPlayTime;
    /* 0x20 */ s64 mPrevPlayTime;
    /* 0x28 */ s64 mLastMeasuredTime;
    /* 0x30 */ s64 mPausedTime;
    /* 0x38 */ u32 _38;
    /* 0x3C */ u32 _3C;
};  // 0x40 bytes per record (the holder's mInfos stride)

class AudRhythmHolder {
public:
    AudRhythmHolder();

    ~AudRhythmHolder(){};

    void movement();
    void copyInfoFromParser();
    void init();
    void copyInfoFromParserOne(u32, u32, u32, s64);

    // Elapsed-time and phase accessors for the selected rhythm record.
    f32 getPlayTimeSec(s32 index);
    f32 getPrevPlayTimeSec(s32 index);
    f32 calcBeatPhase(s32 index);
    f32 calcPrevBeatPhase(s32 index);

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
