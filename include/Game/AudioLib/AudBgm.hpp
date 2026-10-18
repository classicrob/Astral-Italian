#pragma once

#include "Game/AudioLib/AudBgmRhythmStrategy.hpp"
#include "Game/AudioLib/AudTrackController.hpp"
#include "Game/AudioLib/AudBgmVolumeController.hpp"
#include <JSystem/JAudio2/JAISound.hpp>

class JAISoundID;

class AudBgm {
public:
    /// @brief Creates a new `AudBgm`.
    AudBgm();

    virtual void init() = 0;                             // 0x08
    virtual JAISoundHandle* start(u32, bool) = 0;        // 0x0C
    virtual void stop(u32) = 0;                          // 0x10
    virtual bool isPreparedPlay() = 0;                   // 0x14
    virtual s32 playAfterPrepared() = 0;                 // 0x18
    virtual void movement() = 0;                         // 0x1C
    virtual bool moveVolume(f32, u32) = 0;               // 0x20
    virtual bool moveVolumeForNoteFairy(f32, u32) = 0;   // 0x24

    /// @brief Starts the "volume down" fader preset `level` on the volume controller (if the BGM is attached).
    virtual bool volDownWithLevel(s32 level, bool keepIfActive) {  // 0x28
        if (!isAttached()) {
            return false;
        }

        if (mVolumeController != nullptr) {
            mVolumeController->volDownWithLevel(level, keepIfActive);
            return true;
        }

        return false;
    }

    virtual bool moveTempo(f32, s32) = 0;                // 0x2C
    virtual void changeTrackMuteState(s32, s32) = 0;     // 0x30
    virtual JAISoundHandle* getHandle() = 0;             // 0x34
    virtual JAISoundHandle* getRhythmHandle() = 0;       // 0x38
    virtual bool isSoundAttached() const = 0;            // 0x3C
    virtual void pause(bool) = 0;                        // 0x40
    virtual bool isStopping() const = 0;                 // 0x44
    virtual bool isPaused() const = 0;                   // 0x48
    virtual JAISoundID getSoundID() const = 0;           // 0x4C
    virtual JAISoundID getRhythmSoundID() const = 0;     // 0x50
    virtual void moveTrackVolume(f32, s32, s32) = 0;     // 0x54
    virtual u32 getBgmId() const = 0;                    // 0x58
    virtual s32 getMuteStateIndex() const = 0;           // 0x5C
    virtual bool isAttached() const = 0;                 // 0x60

    virtual void setVolumeController(AudBgmVolumeController* pController) {  // 0x64
        mVolumeController = pController;
    }

    virtual AudBgmRhythmStrategy* getRhythmStrategy() {  // 0x68
        return &mRhythmStrategy;
    }

    virtual void sendToSyncStream() = 0;                 // 0x6C
    virtual void rejectFromSyncStream() = 0;             // 0x70

    /// @brief Cancels a pending timed pause.
    virtual void clearPauseFrames() {  // 0x74
        mPauseFrames = -1;
    }

    /// @brief Pauses the BGM and arranges for it to be unpaused after `frames` calls of `updatePauseFrames`.
    virtual void pauseFrames(s32 frames) {  // 0x78
        if (frames > 0) {
            pause(true);
            mPauseFrames = frames;
        }
    }

    /// @brief True when a stream (STM) BGM is attached and not stopping.
    virtual bool isPlayingStream() const {  // 0x7C
        if (isStopping()) {
            return false;
        }

        JAISoundID id = getSoundID();
        return id.getSectionID() == JAISoundID::SOUND_STREAM;
    }

    /// @brief Re-applies the current mute state index with the given fade time.
    virtual void reapplyTrackMuteState(s32 time) {  // 0x80
        changeTrackMuteState(mMuteStateIndex, time);
    }

    virtual void resetInterrupted() {  // 0x84
        if (mVolumeController != nullptr) {
            mVolumeController->resetInterrupted();
        }
    }

    virtual void reset() {  // 0x88
        mVolumeController = nullptr;
        mPauseFrames = -1;
        mMuteStateIndex = 0;
    }

    virtual void resetAuxVolume();  // 0x8C

    virtual void updatePauseFrames() {  // 0x90
        if (mPauseFrames >= 0) {
            if (mPauseFrames == 0) {
                pause(false);
            }

            mPauseFrames--;
        }
    }

    virtual void initTrackMuteState(JAISoundID) = 0;                 // 0x94
    virtual void tryChangeTrackMuteState(s32 track, s32 time) = 0;   // 0x98

    static const s32 mNumTracks = 16;

    /* 0x04 */ AudBgmVolumeController* mVolumeController;
    /* 0x08 */ AudBgmRhythmStrategy mRhythmStrategy;
    /* 0x14 */ s32 mPauseFrames;
    /* 0x18 */ s32 mMuteStateIndex;
};

class AudSingleBgm : public AudBgm {
public:
    /// @brief Creates a new `AudSingleBgm`.
    AudSingleBgm();

    virtual void init();
    virtual JAISoundHandle* start(u32, bool);
    virtual void stop(u32);
    virtual bool isPreparedPlay();
    virtual s32 playAfterPrepared();
    virtual void movement();
    virtual bool moveVolume(f32, u32);
    virtual bool moveVolumeForNoteFairy(f32, u32);
    virtual bool moveTempo(f32, s32);
    virtual void changeTrackMuteState(s32, s32);
    virtual JAISoundHandle* getHandle() {
        return &mHandle;
    }

    virtual JAISoundHandle* getRhythmHandle();
    virtual bool isSoundAttached() const {
        return mHandle.isSoundAttached();
    }

    virtual void pause(bool pause) {
        if (mHandle.isSoundAttached()) {
            mHandle->pause(pause);
        }
    }

    virtual bool isStopping() const {
        if (mHandle.isSoundAttached()) {
            return mHandle->isStopping();
        }

        return true;
    }

    virtual bool isPaused() const {
        if (mHandle.isSoundAttached()) {
            return mHandle->isPaused();
        }

        return false;
    }

    virtual JAISoundID getSoundID() const {
        if (!mHandle.isSoundAttached()) {
            return 0;
        }

        return mHandle->mSoundID;
    }

    virtual JAISoundID getRhythmSoundID() const;
    virtual void moveTrackVolume(f32, s32, s32);

    virtual u32 getBgmId() const {
        return mSoundID;
    }

    virtual s32 getMuteStateIndex() const;

    virtual bool isAttached() const;

    virtual void sendToSyncStream() {
    }

    virtual void rejectFromSyncStream() {
    }

    virtual void initTrackMuteState(JAISoundID);
    virtual void tryChangeTrackMuteState(s32, s32);

    void initTrackController();
    void startTrackControl();
    void stopTrackControl();
    void updateTrackControl();

    /* 0x1C */ JAISoundHandle mHandle;
    /* 0x20 */ JAISoundID mSoundID;
    /* 0x24 */ AudTrackController mTrackController[mNumTracks];
};

class AudMultiBgm : public AudBgm {
public:
    /// @brief Creates a new `AudMultiBgm`.
    AudMultiBgm();

    virtual void init();
    virtual JAISoundHandle* start(u32, bool);
    virtual void stop(u32);
    virtual bool isPreparedPlay();
    virtual s32 playAfterPrepared();
    virtual void movement();
    virtual bool moveVolume(f32, u32);
    virtual bool moveVolumeForNoteFairy(f32, u32);
    virtual bool moveTempo(f32, s32);
    virtual void changeTrackMuteState(s32, s32);
    virtual JAISoundHandle* getHandle();
    virtual JAISoundHandle* getRhythmHandle();
    virtual bool isSoundAttached() const;
    virtual void pause(bool);
    virtual bool isStopping() const;
    virtual bool isPaused() const;
    virtual JAISoundID getSoundID() const;
    virtual JAISoundID getRhythmSoundID() const;
    virtual void moveTrackVolume(f32, s32, s32);
    virtual u32 getBgmId() const;
    virtual s32 getMuteStateIndex() const;

    virtual bool isAttached() const {
        return mHandle.isSoundAttached() && mRhythmHandle.isSoundAttached();
    }

    virtual void sendToSyncStream();
    virtual void rejectFromSyncStream();
    virtual void initTrackMuteState(JAISoundID);
    virtual void tryChangeTrackMuteState(s32, s32);

    void initTrackController();
    void startTrackControl();
    void updateTrackControl();
    JAISoundHandle* prepare(u32);
    bool isPrepared();
    void unlock();
    void updateSyncProcess();
    void pauseSyncProcess();
    void setStreamVolume(f32, f32);

    static const s32 mNumFaders = 2;

    /* 0x01C */ JAISoundHandle mHandle;
    /* 0x020 */ JAISoundHandle mRhythmHandle;
    /* 0x024 */ AudTrackController mTrackController[mNumTracks];
    /* 0x1E4 */ AudFader mFader[mNumFaders];
    /* 0x1FC */ s32 mSyncState;
    /* 0x200 */ s32 mBgmId;
    /* 0x204 */ bool mIsLocked;
};
