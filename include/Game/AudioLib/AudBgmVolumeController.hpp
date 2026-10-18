#pragma once

#include "Game/AudioLib/AudFader.hpp"

#define NUM_FADERS 6

class AudBgmVolumeController {
public:
    /// @brief Creates a new `AudBgmVolumeController`.
    AudBgmVolumeController();

    ~AudBgmVolumeController() {};

    void update();
    f32 getVolume() const;
    void volDown(bool);
    void interruptedByOther();
    void volDownWithLevel(s32, bool);
    void resetInterrupted();
    void moveAuxVolume(f32, s32);
    void moveNoteFairyVolume(f32, s32);

    struct FaderCtrl {
        /* 0x0 */ f32 mVolume;
        /* 0x4 */ s32 mTime;
        /* 0x8 */ s32 mRecoverTime;
    };

    /* 0x00 */ s32 mTimers[NUM_FADERS];
    /* 0x18 */ FaderCtrl mFaderCtrl[NUM_FADERS];
    /* 0x60 */ AudFader mFaders[NUM_FADERS];
    /* 0xA8 */ bool mIsMuted;
};
