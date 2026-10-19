#pragma once
#include <revolution/types.h>
class JAISoundHandle;
struct TamakoroBgmInfo {
    const char* mName;
    u32 mFirstBarSteps;
    u32 mPitchRiseFrames;
    u32 mPitchFallFrames;
    f32 mBaseTempo;
    f32 mMinTempo;
    f32 mMaxTempo;
    f32 mVeryHighTempo;
    f32 mPitchSemitones;
    f32 mUnused;
    f32 mVeryHighPitchSemitones;
    s32 mPercTracks[2];
};
class AudTamakoroBgmCtrl {
public:
    AudTamakoroBgmCtrl();
    void init(const char*);
    void control(f32, bool, s32);
    void modifyTempo(JAISoundHandle*, f32);
    void modifyPitch(JAISoundHandle*, f32, u32);
    bool isPercussionTrack(s32) const NO_INLINE;
    u32 mSteps;
    f32 mTempoRateMin;
    f32 mTempoRateMax;
    f32 mTempoRateVeryHigh;
    f32 mPitchRateNormal;
    f32 mPitchRateVeryHigh;
    u32 mFirstBarSteps;
    const s32* mPercTracks;
    const TamakoroBgmInfo* mInfo;
    s32 mState;
    u32 mPitchFrames;
};
