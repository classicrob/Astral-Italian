#pragma once

#include <JSystem/JAudio2/JAISound.hpp>

namespace AudBgmSetting {
    const char* getBgmParamResourceName();
    void loadBgmParamResource();
    const char* getMultiBgmResourceName();
    void loadMultiBgmResource();
    const char* getStageBgmResourceName();
    void loadStageBgmResource();
    struct BgmSettingInfo {
        /* 0x0 */ u32 mSoundID;
        /* 0x4 */ s32 mMuteStateIndex;
        /* 0x8 */ s32 mExtraChordIndex;
    };

    struct MuteState {
        /* 0x0 */ u8 mMuteStates[8];  // 16 nibbles
        /* 0x8 */ u8 mVolumeA;
        /* 0x9 */ u8 mVolumeB;
    };

    struct MultiBgmSet {
        /* 0x00 */ u32 mSeqId;
        /* 0x04 */ u32 mStreamId;
        /* 0x08 */ f32 mBeatMul;
        /* 0x0C */ u32 mIntroBeats;
        /* 0x10 */ u32 mLoopBeats;
        /* 0x14 */ u32 mLoopStartSamples;  // time (sec) = val / 48000
        /* 0x18 */ u32 mLoopEndSamples;    // time (sec) = val / 48000
    };

    const BgmSettingInfo* findSettingInfo(u32);
    const MuteState* getMuteState(JAISoundID, s32);
    u16 getExtraChordNum(JAISoundID, s32);
    bool hasMuteState(JAISoundID);

};  // namespace AudBgmSetting
