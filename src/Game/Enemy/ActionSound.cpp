#pragma auto_inline off
#include "Game/GameAudio/ActionSound.hpp"
#include "Game/GameAudio/ActionSoundInfo.hpp"
#include "Game/Effect/SyncBckEffectInfo.hpp"
#include "Game/AudioLib/AudSoundObject.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/MathUtil.hpp"

JAISoundHandle* playSoundByType(AudSoundObject* object, s32 type, u32 id, s32 pitch, s32 velocity) {
    JAISoundHandle* handle = nullptr;
    switch (type) {
    case 0: handle = MR::startSoundObjectParam(object, JAISoundID(id), pitch, velocity); break;
    case 2: handle = MR::startSystemSE(JAISoundID(id), pitch, velocity); break;
    case 4: handle = MR::startAtmosphereSE(JAISoundID(id), pitch, velocity); break;
    case 6: MR::startCSSound(id, nullptr, -1, 0, 1.0f); break;
    case 8: MR::fn_800422F0(id); break;
    case 10: handle = MR::startSoundPlayerById(id, pitch, -1); break;
    }
    return handle;
}

JAISoundHandle* playSoundByTypeWithVolume(AudSoundObject* object, s32 type, u32 id, s32 pitch, s32 velocity, s32 volume) {
    JAISoundHandle* handle = nullptr;
    switch (type) {
    case 1: handle = MR::startSoundObjectLevelParam(object, JAISoundID(id), pitch, velocity, volume); break;
    case 3: handle = MR::startSystemLevelSE(JAISoundID(id), pitch, velocity, volume); break;
    case 5: handle = MR::startAtmosphereLevelSE(JAISoundID(id), pitch, velocity); break;
    case 7: MR::startCSSoundAfterPlayed(id, 0); break;
    case 11: handle = MR::startLevelSoundPlayerById(id, pitch); break;
    }
    return handle;
}

void clampValueRange(f32* value, f32 min, f32 max) {
    if (*value < min) *value = min;
    if (*value > max) *value = max;
}

s32 ActionSound::getId() const { return mInfo->mTriggerType; }
const char* ActionSound::getName() const { return mInfo->mName; }
#pragma dont_inline on
f32 ActionSound::getPitchArg(s32 index) const { return mInfo->mPitchArgs[index]; }
f32 ActionSound::getVolumeArg(s32 index) const { return mInfo->mVolumeArgs[index]; }

#pragma dont_inline off

ActionSound::ActionSound(const ActionSoundInfo* info, AudSoundObject* object)
    : mInfo(info), mParam(nullptr), mSoundObject(object), mHandle(nullptr), mUnk10(false) {}

void ActionSound::createSyncBckParam(XanimePlayer* player, ActorAnimKeeper* animKeeper) {
    mParam = new SyncBckEffectInfo("サウンド", player, animKeeper, mInfo->mName, 1, 0.0f, -1.0f, false);
    mParam->mStartFrame = mInfo->mStartFrame;
    mParam->mEndFrame = mInfo->mEndFrame;
}

void ActionSound::markStartPending() {
    if (mInfo->fn_80220830()) mUnk10 = true;
}

void ActionSound::startSoundNow() {
    if (!mInfo->fn_80220830()) {
        mHandle = playSoundByType(mSoundObject, mInfo->mSoundType, mInfo->mSoundId, -1, -1);
        mUnk10 = false;
    }
}

void ActionSound::stopSoundFrames(u32 frames) {
    if (mInfo->fn_80220830()) {
        mUnk10 = false;
        return;
    }
    JAISound* sound = mHandle->mSound;
    s32 type = mInfo->mSoundType;
    if (sound && (type == 0 || type == 2 || type == 4 || type == 10) && mHandle) sound->stop(frames);
}

void ActionSound::releaseActionSoundHandle() {
    if (mInfo->fn_80220830()) {
        mUnk10 = false;
        return;
    }
    JAISoundHandle* handle = mHandle;
    s32 type = mInfo->mSoundType;
    if (handle) {
        JAISound* sound = handle->mSound;
        if (sound && (type == 0 || type == 2 || type == 4 || type == 10) && handle) sound->releaseHandle();
    }
}

void ActionSound::restartSoundWithRate(f32 rate) {
    if (mUnk10) {
        mHandle = playSoundByTypeWithVolume(mSoundObject, mInfo->mSoundType, mInfo->mSoundId, -1, -1, -1);
        applyRateToSound(rate);
    }
}

bool ActionSound::play(s32 mode, s32 pitch, s32 velocity, s32 volume) {
    bool played = false;
    if (mInfo->fn_80220830()) {
        if (mode == 0 || mode == 2) {
            mHandle = playSoundByTypeWithVolume(mSoundObject, mInfo->mSoundType, mInfo->mSoundId, pitch, velocity, volume);
            played = true;
        }
    } else if (mode == 0 || mode == 1) {
        mHandle = playSoundByType(mSoundObject, mInfo->mSoundType, mInfo->mSoundId, pitch, velocity);
        played = true;
    }
    applyPitchVolumeArgs(pitch, velocity);
    return played;
}

void ActionSound::applyRateToSound(f32 rate) {
    if (mHandle) {
        if (mInfo->mPitchType == 0) {
            f32 scale = getPitchArg(0);
            f32 offset = getPitchArg(1);
            f32 min = getPitchArg(2);
            f32 max = getPitchArg(3);
            f32 pitch = 1.0f + (offset + (rate - 1.0f) * scale);
            clampValueRange(&pitch, min, max);
            MR::clampBoth(&pitch, 0.01f, 10.0f);
            mHandle->mSound->mParams.mMove.mParams.mPitch *= pitch;
        }
        if (mInfo->mVolumeType == 0) {
            f32 scale = getVolumeArg(0);
            f32 offset = getVolumeArg(1);
            f32 min = getVolumeArg(2);
            f32 max = getVolumeArg(3);
            f32 volume = 1.0f + (offset + (rate - 1.0f) * scale);
            clampValueRange(&volume, min, max);
            MR::clampBoth(&volume, 0.0f, 2.0f);
            mHandle->mSound->mParams.mMove.mParams.mVolume *= volume;
        }
    }
}

// NONMATCHING: optimizer-disabled stack-base register and epilogue scheduling differ; branch graph and arithmetic match.
#pragma global_optimizer off
void ActionSound::applyPitchVolumeArgs(s32 pitchArg, s32 volumeArg) {
    if (mHandle && mHandle->mSound) {
        f32 pitch = 0.0f;
        s32 pitchType = mInfo->mPitchType;
        if (pitchType == -1) goto volume;
        if (pitchType == 1) goto pitch1;
        if (pitchType == 2) goto pitch2;
        goto volume;
    pitch1: {
        f32 scale = getPitchArg(0);
        f32 offset = getPitchArg(1);
        f32 min = getPitchArg(2);
        f32 max = getPitchArg(3);
        pitch = offset + pitchArg * scale;
        clampValueRange(&pitch, min, max);
        MR::clampBoth(&pitch, 0.01f, 10.0f);
        mHandle->mSound->mParams.mMove.mParams.mPitch *= pitch;
        goto volume;
    }
    pitch2: {
        f32 scale = getPitchArg(0);
        f32 offset = getPitchArg(1);
        f32 min = getPitchArg(2);
        f32 max = getPitchArg(3);
        pitch = offset + volumeArg * scale;
        clampValueRange(&pitch, min, max);
        MR::clampBoth(&pitch, 0.01f, 10.0f);
        mHandle->mSound->mParams.mMove.mParams.mPitch *= pitch;
        goto volume;
    }
    volume:
        f32 volume = 0.0f;
        s32 volumeType = mInfo->mVolumeType;
        if (volumeType == -1) goto done;
        if (volumeType == 1) goto volume1;
        if (volumeType == 2) goto volume2;
        goto done;
    volume1: {
        f32 scale = getVolumeArg(0);
        f32 offset = getVolumeArg(1);
        f32 min = getVolumeArg(2);
        f32 max = getVolumeArg(3);
        volume = offset + pitchArg * scale;
        clampValueRange(&volume, min, max);
        MR::clampBoth(&volume, 0.0f, 2.0f);
        mHandle->mSound->mParams.mMove.mParams.mVolume *= volume;
        goto done;
    }
    volume2: {
        f32 scale = getVolumeArg(0);
        f32 offset = getVolumeArg(1);
        f32 min = getVolumeArg(2);
        f32 max = getVolumeArg(3);
        volume = offset + volumeArg * scale;
        clampValueRange(&volume, min, max);
        MR::clampBoth(&volume, 0.0f, 2.0f);
        mHandle->mSound->mParams.mMove.mParams.mVolume *= volume;
    }
    }
done:
    return;
}
#pragma global_optimizer on
