#pragma once

#include "Game/AudioLib/AudRemixSequencer.hpp"
#include "Game/AudioLib/AudSoundObject.hpp"

class JKRHeap;

struct RemixNoteData {
    /* 0x0 */ s32 mPitch;
    /* 0x4 */ s32 mVelocity;
    /* 0x8 */ s32 mLength;
    /* 0xC */ s32 mStartTime;
};

struct RemixNoteTrackData {
    /* 0x0 */ s32 mTrackNo;
    /* 0x4 */ RemixNoteData* mNotes;
};

struct RemixNoteGroupData {
    /* 0x0  */ s32 mIndex;
    /* 0x4  */ s32 mTrackCount;
    /* 0x8  */ s32 mNoteCount;
    /* 0xC  */ u32* mNoteTimings;
    /* 0x10 */ RemixNoteTrackData* mRemixTracks;
};

class AudRemixMgr {
public:
    AudRemixMgr(JKRHeap* pHeap);
    void init();
    void update();
    void setRemixSeqResource(void* ptr);
    RemixNoteGroupData* getRemixNoteGroupDataFromMelodyNo(s32 melodyNo) const;

    /* 0x0  */ JKRHeap* mHeap;
    /* 0x4  */ RemixNoteGroupData* mGroups;
    /* 0x8  */ s32 mGroupCount;
    /* 0xC  */ AudSoundObject* mSoundObj;
    /* 0x10 */ AudRemixSequencer* mRemixSeq;
};
