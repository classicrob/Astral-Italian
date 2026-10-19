#pragma once

#include <revolution/types.h>

class JAISoundID;
class RemixNoteData;
class RemixNoteTrackData;

class AudRmxSeqNoteOnTimer {
public:
    AudRmxSeqNoteOnTimer();

    void initData();
    void setData(const RemixNoteTrackData*, const RemixNoteData*);
    bool update(f32);
    JAISoundID getFreeSeID();

    /* 0x0 */ f32 mStartDelay;
    /* 0x4 */ f32 mRemaining;
    /* 0x8 */ const RemixNoteData* mNote;
    /* 0xC */ const RemixNoteTrackData* mTrack;
};

class AudRemixSequencer {
public:
    AudRemixSequencer();

    void initNoteOnBuff();
    void update();
    void setTempo(f32) NO_INLINE;
    AudRmxSeqNoteOnTimer* newNoteOnTimer();
    void addNoteData(const RemixNoteTrackData*, const RemixNoteData*);

    /* 0x000 */ AudRmxSeqNoteOnTimer mTimers[0x20];
    /* 0x200 */ f32 mTempo;
    /* 0x204 */ f32 mStepPerFrame;
};
