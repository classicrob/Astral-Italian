#define JAISOUND_STOPPING_NOINLINE

#include "Game/AudioLib/AudBgm.hpp"
#include "Game/AudioLib/AudBgmMgr.hpp"
#include "Game/AudioLib/AudBgmSetting.hpp"
#include "Game/AudioLib/AudBgmVolumeController.hpp"
#include "Game/AudioLib/AudFader.hpp"
#include "Game/AudioLib/AudSoundId.hpp"
#include "Game/AudioLib/AudSoundInfo.hpp"
#include "Game/AudioLib/AudSystem.hpp"
#include "Game/AudioLib/AudWrap.hpp"
#include "Game/RhythmLib/AudBgmTempoAdjuster.hpp"
#include "Game/RhythmLib/AudRhythmMeSystem.hpp"
#include "Game/RhythmLib/AudRhythmSeqParser.hpp"
#include <JSystem/JAudio2/JAISound.hpp>
#include <JSystem/JAudio2/JAISoundChild.hpp>
#include <JSystem/JAudio2/JAISoundInfo.hpp>
#include <JSystem/JAudio2/JASCriticalSection.hpp>
#include "Game/RhythmLib/AudMePlayer.hpp"
#include "Game/RhythmLib/AudMeTrack.hpp"

AudSingleBgm::AudSingleBgm() : AudBgm(), mHandle(), mSoundID(-1) {
    init();
}

void AudSingleBgm::init() {
    mSoundID.setAnonymous();
    initTrackController();
}

void AudSingleBgm::movement() {
    updateTrackControl();
    if (!mHandle.isSoundAttached()) {
        resetAuxVolume();
        return;
    }

    if (mVolumeController != nullptr) {
        mHandle->getAuxiliary().moveVolume(mVolumeController->getVolume(), 0);
    }
}

JAISoundHandle* AudSingleBgm::start(u32 soundID, bool lock) {
    JAISoundID id = soundID;
    mSoundID = soundID;

    if (mHandle.isSoundAttached()) {
        mHandle->stop();
    }

    resetAuxVolume();

    if (id.getSectionID() == 0x2) {  // STM
        AudWrap::getSystem()->getStreamMgr().startSound(id, &mHandle, nullptr);
    } else {  // BGM or MBGM
        AudSoundInfo* info = AudWrap::getSoundInfo();
        AudWrap::getSystem()->getSeqMgr().startSound(id, &mHandle, nullptr);
        if (mHandle.isSoundAttached()) {
            startTrackControl();
            changeTrackMuteState(0, 0);
        }
    }

    if (mHandle.isSoundAttached()) {
        if (mVolumeController != nullptr) {
            mHandle->getAuxiliary().moveVolume(mVolumeController->getVolume(), 0);
        }

        if (lock) {
            mHandle->lockWhenPrepared();
        }
    }

    return &mHandle;
}

void AudSingleBgm::stop(u32 time) {
    if (!mHandle.isSoundAttached()) {
        return;
    }

    mHandle->stop(time);

    if (mSoundID.getSectionID() != 0x2) {  // not STM
        stopTrackControl();
    }
}

bool AudSingleBgm::isPreparedPlay() {
    if (mHandle.isSoundAttached()) {
        return mHandle->isPrepared();
    }

    return false;
}

s32 AudSingleBgm::playAfterPrepared() {
    if (mHandle.isSoundAttached()) {
        return mHandle->unlockIfLocked();
    }

    return 0;
}

JAISoundHandle* AudSingleBgm::getRhythmHandle() {
    if (mHandle.isSoundAttached()) {
        if (mHandle->getID().getSectionID() == 0x1) {  //  BGM or MBGM
            return &mHandle;
        }
    }

    return nullptr;
}

bool AudSingleBgm::moveVolume(f32 volume, u32 time) {
    if (!mHandle.isSoundAttached()) {
        return false;
    }

    if (mVolumeController != nullptr) {
        mVolumeController->moveAuxVolume(volume, time);
        return true;
    }

    return false;
}

bool AudSingleBgm::moveVolumeForNoteFairy(f32 volume, u32 time) {
    if (!mHandle.isSoundAttached()) {
        return false;
    }

    if (mVolumeController != nullptr) {
        mVolumeController->moveNoteFairyVolume(volume, time);
        return true;
    }

    return false;
}

bool AudSingleBgm::moveTempo(f32 tempo, s32 time) {
    if (!mHandle.isSoundAttached()) {
        return false;
    }

    JAITempoMgr* pTempoMgr = mHandle->getTempoMgr();
    if (pTempoMgr != nullptr) {
        if (time == 0) {
            pTempoMgr->setTempo(tempo);
        } else {
            pTempoMgr->mTransition.set(tempo, pTempoMgr->mTempo, time);
        }

        return true;
    }

    return false;
}

void AudSingleBgm::initTrackMuteState(JAISoundID soundID) {
    AudSoundInfo* info = AudWrap::getSoundInfo();
    bool isRhythm = info->isRhythmBgm(soundID);

    for (s32 i = 0; i < mNumTracks; i++) {
        if (i != 0 || !isRhythm) {
            mTrackController[i].setMuteState(AudFader::FadeState_FadeIn, 0, false);
        }
    }

    mMuteStateIndex = 0;
}

void AudSingleBgm::tryChangeTrackMuteState(s32 track, s32 time) {
    if (AudBgmSetting::getMuteState(mSoundID, track) != nullptr) {
        changeTrackMuteState(track, time);
    }
}

s32 AudSingleBgm::getMuteStateIndex() const {
    if (AudBgmSetting::hasMuteState(mSoundID)) {
        return mMuteStateIndex;
    }

    return -1;
}

void AudSingleBgm::changeTrackMuteState(s32 track, s32 time) {
    if (mSoundID.getSectionID() == 0x2) {  // STM
        return;
    }

    AudSoundInfo* info = AudWrap::getSoundInfo();
    bool isRhythm = info->isRhythmBgm(mSoundID);

    const AudBgmSetting::MuteState* muteState = AudBgmSetting::getMuteState(mSoundID, track);
    if (muteState == nullptr) {
        for (s32 i = 0; i < mNumTracks; i++) {
            if (i != 0 || !isRhythm) {
                mTrackController[i].setMuteState(AudFader::FadeState_FadeIn, time, false);
            }
        }

        return;
    }

    for (s32 i = 0; i < mNumTracks; i++) {
        u8 state = muteState->mMuteStates[i / 2];
        if (i % 2 == 0) {
            state >>= 4;
        }

        bool autoMute = false;
        if ((state & 0x8) > 0) {
            autoMute = true;
        }

        mTrackController[i].setMuteState(state & 0x7, time, autoMute);
    }
}

void AudSingleBgm::moveTrackVolume(f32 volume, s32 track, s32 time) {
    mTrackController[track].moveVolume(volume, time);
}

void AudSingleBgm::initTrackController() {
    for (s32 i = 0; i < mNumTracks; i++) {
        mTrackController[i].mTrackNo = i;
    }
}

void AudSingleBgm::startTrackControl() {
    for (s32 i = 0; i < mNumTracks; i++) {
        mTrackController[i].start(&mHandle);
    }
}

void AudSingleBgm::stopTrackControl() {
    for (s32 i = 0; i < mNumTracks; i++) {
        mTrackController[i].stop();
    }
}

void AudSingleBgm::updateTrackControl() {
    if (mSoundID.getSectionID() == 0x2) {  // STM
        return;
    }

    for (s32 i = 0; i < mNumTracks; i++) {
        mTrackController[i].update();
    }
}

JAISoundID AudSingleBgm::getRhythmSoundID() const {
    return getSoundID();
}

bool AudSingleBgm::isAttached() const {
    return mHandle.isSoundAttached();
}

// NONMATCHING (data order only): the target's two init guards sit in reverse order in .sbss (AudMe at 0x..29).
// The target's static initializer of this unit constructs the memory pools of AudMe and AudMeTrack.
template JASMemPool< AudMe > JASPoolAllocObject< AudMe >::memPool_;
template JASMemPool< AudMeTrack > JASPoolAllocObject< AudMeTrack >::memPool_;
