#include "Game/GameAudio/AudTamakoroBgmCtrl.hpp"
#include "Game/AudioLib/AudSoundId.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include <JSystem/JAudio2/JAISoundChild.hpp>
#include <JSystem/JAudio2/JASCriticalSection.hpp>
#include "Game/Util/StringUtil.hpp"
#include <cmath>

namespace {
    f32 sSpeedMin = 2.0f;
    f32 sSpeedMax = 22.0f;
    f32 sSpeedLowThreshold = 7.0f;
    f32 sSpeedMidThreshold = 12.0f;
    s32 sStateNumLow = 1;
    s32 sStateNumMid = 2;
    s32 sStateNumHigh = 3;
    s32 sStateNumVeryHigh = 4;
    const TamakoroBgmInfo sBgmInfo[] = {
        {"BGM_TAMAKORO",107,30,30,134.0f,73.70000457763672f,163.48001098632812f,198.052001953125f,0.0f,0.0f,2.9969849586486816f,{14,-1}},
        {"BGM_TAMAKORO_2",202,30,15,84.0f,80.0f,92.0f,112.0f,0.0f,0.0f,4.996984958648682f,{14,15}},
    };
    s32 sBgmInfoCount = 2;
};  // namespace

AudTamakoroBgmCtrl::AudTamakoroBgmCtrl()
    : mSteps(0), mTempoRateMin(0.0f), mTempoRateMax(0.0f), mTempoRateVeryHigh(0.0f), mPitchRateNormal(0.0f), mPitchRateVeryHigh(0.0f),
      mFirstBarSteps(0), mPercTracks(nullptr), mInfo(nullptr), mState(-1), mPitchFrames(0) {
}

// NONMATCHING: equivalent table-search loop layout/regalloc
enum {
    TAMAKORO_BGM_ID = 0x1000009,
    TAMAKORO_BGM_2_ID = 0x100000A,
};

namespace {
    inline const TamakoroBgmInfo* findBgmInfo(const char* name) {
        for (s32 i = 0; i < sBgmInfoCount; ++i) {
            if (MR::isEqualString(sBgmInfo[i].mName, name)) {
                return &sBgmInfo[i];
            }
        }
        return nullptr;
    }
}

void AudTamakoroBgmCtrl::init(const char* name) {
    mSteps = 0;
    mInfo = findBgmInfo(name);
    mTempoRateMin = mInfo->mMinTempo / mInfo->mBaseTempo;
    mTempoRateMax = mInfo->mMaxTempo / mInfo->mBaseTempo;
    mTempoRateVeryHigh = mInfo->mVeryHighTempo / mInfo->mBaseTempo;
    mPitchRateNormal = ::pow(2.0, mInfo->mPitchSemitones / 12.0f);
    mPitchRateVeryHigh = ::pow(2.0, mInfo->mVeryHighPitchSemitones / 12.0f);
    mPitchFrames = mInfo->mPitchRiseFrames;
    mFirstBarSteps = mInfo->mFirstBarSteps;
    mPercTracks = mInfo->mPercTracks;
}

// NONMATCHING: register scheduling
void AudTamakoroBgmCtrl::control(f32 speed, bool disabled, s32 frames) {
    AudBgm* bgm = AudWrap::getStageBgm();
    if (bgm != nullptr) {
        JAISoundHandle* handle = bgm->getHandle();
        if (handle != nullptr && handle->isSoundAttached()) {
            JAISoundID id = handle->getSound()->getID();
            // The target compares against indices 9 and 10 of the BGM section (0x1000009 / 0x100000A); AudSoundId.hpp lists
            // BGM_TAMAKORO at 0x100004B, so the shared enum disagrees with the code here.
            if (id == TAMAKORO_BGM_ID || id == TAMAKORO_BGM_2_ID) {
                if (mSteps > mFirstBarSteps) {
                    if (speed < ::sSpeedMin) {
                        speed = ::sSpeedMin;
                    }
                    if (speed > ::sSpeedMax) {
                        speed = ::sSpeedMax;
                    }
                    f32 t0;
                    f32 t1;
                    s32 stateNum;
                    f32 tempoRate;
                    f32 pitchRate;
                    if (disabled) {
                        stateNum = ::sStateNumVeryHigh;
                        tempoRate = mTempoRateVeryHigh;
                        pitchRate = mPitchRateVeryHigh;
                    } else {
                        if (speed < ::sSpeedLowThreshold) {
                            stateNum = ::sStateNumLow;
                        } else if (speed < ::sSpeedMidThreshold) {
                            stateNum = ::sStateNumMid;
                        } else {
                            stateNum = ::sStateNumHigh;
                        }

                        t0 = speed - ::sSpeedMin;
                        t1 = ::sSpeedMax - ::sSpeedMin;
                        tempoRate = mTempoRateMin + (t0 / t1) * (mTempoRateMax - mTempoRateMin);
                        pitchRate = mPitchRateNormal;
                    }
                    if (mState != stateNum) {
                        if (mState < stateNum) mPitchFrames = mInfo->mPitchRiseFrames;
                        else mPitchFrames = mInfo->mPitchFallFrames;
                        mState = stateNum;
                    }
                    bgm->changeTrackMuteState(stateNum, frames);
                    modifyTempo(handle, tempoRate);
                    modifyPitch(handle, pitchRate, mPitchFrames);
                }
                mSteps++;
            }
        }
    }
}

void AudTamakoroBgmCtrl::modifyTempo(JAISoundHandle* pHandle, f32 tempo) {
    if (pHandle != nullptr && pHandle->isSoundAttached()) {
        JAITempoMgr* mgr = pHandle->getSound()->getTempoMgr();
        if (mgr != nullptr) {
            mgr->mTransition.set(tempo, mgr->getTempo(), 10);
        }
    }
}

void AudTamakoroBgmCtrl::modifyPitch(JAISoundHandle* pHandle, f32 pitch, u32 frames) {
    if (pHandle != nullptr && pHandle->isSoundAttached()) {
        for (s32 i = 0; i < 16; i++) {
            if (isPercussionTrack(i)) {
                continue;
            }
            JASCriticalSection crit;
            JAISoundChild* child = pHandle->getSound()->getChild(i);
            if (child == nullptr) {
                continue;
            }
            child->mMove.movePitch(pitch, frames);
        }
    }
}

bool AudTamakoroBgmCtrl::isPercussionTrack(s32 track) const {
    for (s32 i = 0; i < 2; ++i) if (mPercTracks[i] == track) return true;
    return false;
}
