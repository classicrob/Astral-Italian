#pragma once

#ifdef GALAXY_NATIVE
#include "Game/Util/BigEndian.hpp"
#endif

#include <JSystem/JAudio2/JASGlobal.hpp>
#include <JSystem/JAudio2/JASHeapCtrl.hpp>
#include <JSystem/JGeometry/TVec.hpp>
#include <JSystem/JSupport/JSUList.hpp>

class AudMeHandle;
class AudMeTrack;

struct AudMePlayingParams {
    /* 0x0 */ u8 mPriority;
    /* 0x1 */ u8 mChordVolume;
    /* 0x2 */ u8 mPan;
    /* 0x3 */ u8 mDolby;
#ifdef GALAXY_NATIVE
    // Read in place from the big-endian ME table resource.
    /* 0x4 */ BigEndianValue< u16 > mStopTime;
#else
    /* 0x4 */ u16 mStopTime;
#endif
    /* 0x6 */ u8 _6;
    /* 0x7 */ u8 _7;
};

class AudMePlayingParamsHolder {
public:
    AudMePlayingParamsHolder() : mNumEntries(), mParams(), mNames() {
    }

    void setResource(void*);

    /* 0x0 */ u32 mNumEntries;
    /* 0x4 */ AudMePlayingParams* mParams;
    /* 0x8 */ const char** mNames;
};

class AudMe : public JSULink< AudMe >, public JASPoolAllocObject< AudMe > {
public:
    static const s32 TIMED_PARAMS = 6;

    enum MoveParam {
        MoveParam_Volume = 0,
        MoveParam_Pitch = 1,
        MoveParam_FxMix = 2,
        MoveParam_Pan = 3,
        MoveParam_Dolby = 4,
        MoveParam_DistFilter = 5,
    };

    struct MoveParam_ {
        /* 0x0 */ f32 mValue;
        /* 0x4 */ f32 mTarget;
        /* 0x8 */ f32 mDelta;
    };

    AudMe();

    ~AudMe();

    void requestStart(void*, u32);
    bool start(void*, u32);
    void stop(s32);
    void release();
    void attachHandle(AudMeHandle*);
    void releaseHandle();
    void update();
    void setTimedParam(s32, f32, u32);
    void initTimed();
    void updateTimedParams(bool);
    void setOuterPlayingParams(f32, f32, f32, s32, s32);
    void doneStop();
    void setPauseFlag(bool);

    /* 0x10 */ AudMeHandle* mHandle;
    /* 0x14 */ AudMeTrack* mTrack;
    /* 0x18 */ u32 _18;
    /* 0x1C */ const TVec3f* mPos;
    /* 0x20 */ s32 mMeId;
    /* 0x24 */ s32 mStopTimer;
    /* 0x28 */ u32 mTimer;  // mTimer
    /* 0x2C */ MoveParam_ mMoveParams[TIMED_PARAMS];  // indexed by the MoveParam_* enum
    /* 0x74 */ s32 mStartRequested;
    /* 0x78 */ void* mData;
    /* 0x7C */ u32 mSeqStartPos;
    /* 0x80 */ f32 mMasterVolume;
    /* 0x84 */ f32 mVolume;
    /* 0x88 */ f32 mOuterPan;
    /* 0x8C */ f32 mOuterDolby;
    /* 0x90 */ s32 mPriority;
    /* 0x94 */ s32 mDefaultStopTime;
};

struct AudMeTable {
#ifdef GALAXY_NATIVE
    /* 0x0 */ BigEndianValue< s32 > mNumEntries;
    /* 0x4 */ BigEndianValue< s32 > mSeqStartPos[1];
#else
    /* 0x0 */ s32 mNumEntries;
    /* 0x4 */ s32 mSeqStartPos[1];
#endif
};

class AudMeMgr : public JASGlobalInstance< AudMeMgr > {
public:
    AudMeMgr();

    void setMeSeq(void*);
    void setMeTable(void*);
    bool startMe(u32, AudMeHandle*, const TVec3f*);
    void freeDeadMe();
    void updateEachMe();
    void stopAll(s32);
    void setPauseFlagAll(bool);
    void update();
    s32 getSeqStartPos(u32);
    bool isRequestedMe(u32);
    void setOuterPlayingParam(s32, AudMe*);

    u8 getPriority(int index) {
        return mParams.mParams[index].mPriority;
    }

    /* 0x00 */ AudMeTable* mMeSeq;
    /* 0x04 */ JSUList< AudMe > mMeList;
    /* 0x10 */ u8 _10[64];  // mNotes?
    /* 0x50 */ f32 mMasterVolume;
    /* 0x54 */ AudMePlayingParamsHolder mParams;
};
