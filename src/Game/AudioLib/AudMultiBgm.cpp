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

AudMultiBgm::AudMultiBgm() : AudBgm(), mHandle(), mRhythmHandle(), mSyncState(), mBgmId(-1), mIsLocked() {
    init();
}

void AudMultiBgm::init() {
    initTrackController();
    for (s32 i = 0; i < mNumFaders; i++) {
        mFader[i].set(1.0f, 0);
    }
}

JAISoundHandle* AudMultiBgm::start(u32 soundID, bool lock) {
    if (!isStopping()) {
        stop(0);
    }

    mIsLocked = lock;
    return prepare(soundID);
}

void AudMultiBgm::stop(u32 time) {
    if (mRhythmHandle.isSoundAttached()) {
        mRhythmHandle->stop(time);
    }

    if (mHandle.isSoundAttached()) {
        mHandle->stop(time);
    }

    mSyncState = 0;
    mBgmId = -1;
}

bool AudMultiBgm::isPreparedPlay() {
    return isPrepared();
}

s32 AudMultiBgm::playAfterPrepared() {
    bool wasLocked = mIsLocked;
    mIsLocked = false;
    return wasLocked;
}

void AudMultiBgm::sendToSyncStream() {
    if (!mRhythmHandle.isSoundAttached()) {
        return;
    }

    if (!mHandle.isSoundAttached()) {
        return;
    }

    AudRhythmMeSystem* meSystem = AudRhythmMeSystem::getInstance();
    s32 idx = mRhythmStrategy.mBgmIdx;
    if (idx >= 0) {
        AudBgmTempoAdjuster* adjuster = meSystem->getSeqParser(idx)->getTempoAdjuster();
        f32 beatMul = AudBgmSetting::getBeatMulForMultiBgm(mBgmId);
        u32 introBeats = AudBgmSetting::getIntroBeatsForMultiBgm(mBgmId);
        u32 loopBeats = AudBgmSetting::getLoopBeatsForMultiBgm(mBgmId);
        u32 loopStartSamples = AudBgmSetting::getLoopStartSamplesForMultiBgm(mBgmId);
        u32 loopEndSamples = AudBgmSetting::getLoopEndSamplesForMultiBgm(mBgmId);
        JAIStream* stream = mHandle->asStream();

        JASCriticalSection crit;
        adjuster->setInfo(beatMul, introBeats, loopBeats, loopStartSamples, loopEndSamples);
        adjuster->setStream(stream);
    }
}

void AudMultiBgm::rejectFromSyncStream() {
    AudRhythmMeSystem* meSystem = AudRhythmMeSystem::getInstance();
    s32 idx = mRhythmStrategy.mBgmIdx;
    if (idx >= 0) {
        JASCriticalSection crit;
        AudBgmTempoAdjuster* adjuster = meSystem->getSeqParser(idx)->getTempoAdjuster();
        adjuster->reject();
    }
}

void AudMultiBgm::movement() {
    updateSyncProcess();
    updateTrackControl();

    for (s32 i = 0; i < mNumFaders; i++) {
        mFader[i].update();
    }

    f32 volume = 1.0f;
    if (mVolumeController != nullptr) {
        volume = mVolumeController->getVolume();
    }

    if (!mHandle.isSoundAttached()) {
        if (!mRhythmHandle.isSoundAttached()) {
            resetAuxVolume();
            if (mVolumeController != nullptr) {
                for (s32 i = 0; i < mNumFaders; i++) {
                    mFader[i].set(1.0f, 0);
                }
            }
        }
    } else {
        f32 volA = volume * mFader[0].getVolume();
        f32 volB = volume * mFader[1].getVolume();
        setStreamVolume(volA, volB);
    }

    if (mRhythmHandle.isSoundAttached()) {
        mRhythmHandle->getAuxiliary().moveVolume(volume, 0);
    }
}

bool AudMultiBgm::moveVolume(f32 volume, u32 time) {
    if (!mHandle.isSoundAttached() && !mRhythmHandle.isSoundAttached()) {
        return false;
    }

    if (mVolumeController != nullptr) {
        mVolumeController->moveAuxVolume(volume, time);
        return true;
    }

    return false;
}

bool AudMultiBgm::moveVolumeForNoteFairy(f32 volume, u32 time) {
    if (!mHandle.isSoundAttached() && !mRhythmHandle.isSoundAttached()) {
        return false;
    }

    if (mVolumeController != nullptr) {
        mVolumeController->moveNoteFairyVolume(volume, time);
        return true;
    }

    return false;
}

bool AudMultiBgm::moveTempo(f32, s32) {
    return false;
}

void AudMultiBgm::initTrackMuteState(JAISoundID soundID) {
    AudSoundInfo* info = AudWrap::getSoundInfo();
    bool isRhythm = info->isRhythmBgm(soundID);

    for (s32 i = 0; i < mNumTracks; i++) {
        if (i != 0 || !isRhythm) {
            mTrackController[i].setMuteState(AudFader::FadeState_FadeIn, 0, false);
        }
    }

    mMuteStateIndex = 0;
}

void AudMultiBgm::tryChangeTrackMuteState(s32 track, s32 time) {
    if (AudBgmSetting::getMuteState(mRhythmHandle->getID(), track) != nullptr) {
        changeTrackMuteState(track, time);
    }
}

s32 AudMultiBgm::getMuteStateIndex() const {
    if (AudBgmSetting::hasMuteState(mRhythmHandle->getID())) {
        return mMuteStateIndex;
    }

    return -1;
}

void AudMultiBgm::changeTrackMuteState(s32 track, s32 time) {
    if (!mRhythmHandle.isSoundAttached()) {
        return;
    }

    JAISoundID soundID = mRhythmHandle->getID();

    if (soundID.getSectionID() == 0x2) {  // STM
        return;
    }

    AudSoundInfo* info = AudWrap::getSoundInfo();
    bool isRhythm = info->isRhythmBgm(soundID);

    const AudBgmSetting::MuteState* muteState = AudBgmSetting::getMuteState(soundID, track);
    if (muteState == nullptr) {
        for (s32 i = 0; i < mNumTracks; i++) {
            if (i != 0 || !isRhythm) {
                mTrackController[i].setMuteState(AudFader::FadeState_FadeIn, time, false);
            }
        }

        return;
    }

    switch (soundID) {
    case BGM_GALAXY_01:
        if (track == 1) {
            time *= 3;
        }

        break;
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

    u8 volA = muteState->mVolumeA;
    if (volA != 0xFF) {
        mFader[0].set(volA / 254.0f, time);
    }

    u8 volB = muteState->mVolumeB;
    if (volB != 0xFF) {
        mFader[1].set(volB / 254.0f, time);
    }

    if (track != 0) {
        u16 extraChordNum = AudBgmSetting::getExtraChordNum(soundID, track);
        if (extraChordNum != 0xFFFF) {
            AudWrap::getSystem()->getChordInfo()->loadChordInfo(extraChordNum, true);
        }
    }
}

void AudMultiBgm::moveTrackVolume(f32 volume, s32 track, s32 time) {
    mTrackController[track].moveVolume(volume, time);
}

JAISoundHandle* AudMultiBgm::getHandle() {
    return &mHandle;
}

JAISoundHandle* AudMultiBgm::getRhythmHandle() {
    return &mRhythmHandle;
}

bool AudMultiBgm::isSoundAttached() const {
    return mHandle.isSoundAttached();
}

void AudMultiBgm::pause(bool pause) {
    if (mHandle.isSoundAttached()) {
        mHandle->pause(pause);
    }
}

bool AudMultiBgm::isStopping() const {
    if (mHandle.isSoundAttached()) {
        return mHandle->isStopping();
    }

    return true;
}
