#pragma once

#include <revolution.h>

class ActionSoundInfo;
class SyncBckEffectInfo;
class XanimePlayer;
class JAISoundHandle;
class AudSoundObject;

class ActionSound {
public:
    ActionSound(const ActionSoundInfo* pInfo, AudSoundObject* pSoundObject);

    void createSyncBckParam(XanimePlayer*, s32);
    void markStartPending();
    void startSoundNow();
    void stopSoundFrames(u32);
    void releaseActionSoundHandle();
    void restartSoundWithRate(f32);
    void applyRateToSound(f32);
    void applyPitchVolumeArgs(s32, s32);
    f32 getPitchArg(s32) const;
    f32 getVolumeArg(s32) const;
    s32 getId() const;
    const char* getName() const;
    bool play(s32 mode, s32 pitch, s32 velocity, s32 volume);

    const ActionSoundInfo* mInfo;
    SyncBckEffectInfo* mParam;
    AudSoundObject* mSoundObject;
    JAISoundHandle* mHandle;
    bool mUnk10;
};

JAISoundHandle* playSoundByType(AudSoundObject*, s32, u32, s32, s32);
JAISoundHandle* playSoundByTypeWithVolume(AudSoundObject*, s32, u32, s32, s32, s32);
void clampValueRange(f32*, f32, f32);
