#pragma auto_inline off
#include "Game/GameAudio/ActionSound.hpp"
#include "Game/GameAudio/ActionSoundInfo.hpp"
#include "Game/Effect/SyncBckEffectInfo.hpp"
#include "Game/AudioLib/AudSoundObject.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/MathUtil.hpp"

JAISoundHandle* fn_8021F6F0(AudSoundObject* object, s32 type, u32 id, s32 pitch, s32 velocity) {
    JAISoundHandle* handle = nullptr;
    switch (type) {
    case 0: handle = MR::startSoundObjectParam(object, JAISoundID(id), pitch, velocity); break;
    case 2: handle = MR::startSystemSE(JAISoundID(id), pitch, velocity); break;
    case 4: handle = MR::startAtmosphereSE(JAISoundID(id), pitch, velocity); break;
    case 6: MR::startCSSound(id, nullptr, -1, 0, 1.0f); break;
    case 8: MR::fn_800422F0(id); break;
    case 10: handle = MR::fn_80050D10(id, pitch, -1); break;
    }
    return handle;
}

JAISoundHandle* fn_8021F7E0(AudSoundObject* object, s32 type, u32 id, s32 pitch, s32 velocity, s32 volume) {
    JAISoundHandle* handle = nullptr;
    switch (type) {
    case 1: handle = MR::startSoundObjectLevelParam(object, JAISoundID(id), pitch, velocity, volume); break;
    case 3: handle = MR::fn_8005AF70(JAISoundID(id), pitch, velocity, volume); break;
    case 5: handle = MR::fn_8005B100(JAISoundID(id), pitch, velocity); break;
    case 7: MR::startCSSoundAfterPlayed(id, 0); break;
    case 11: handle = MR::fn_80050E60(id, pitch); break;
    }
    return handle;
}

void fn_8021F8C0(f32* value, f32 min, f32 max) {
    if (*value < min) *value = min;
    if (*value > max) *value = max;
}

s32 ActionSound::getId() const { return mInfo->mTriggerType; }
const char* ActionSound::getName() const { return mInfo->mName; }
#pragma dont_inline on
f32 ActionSound::fn_8021F910(s32 index) const { return mInfo->mPitchArgs[index]; }
f32 ActionSound::fn_8021F930(s32 index) const { return mInfo->mVolumeArgs[index]; }

#pragma dont_inline off

ActionSound::ActionSound(const ActionSoundInfo* info, AudSoundObject* object)
    : mInfo(info), mParam(nullptr), mSoundObject(object), mHandle(nullptr), mUnk10(false) {}

void ActionSound::fn_8021F970(XanimePlayer* player, s32 index) {
    mParam = new SyncBckEffectInfo("サウンド", player, index, mInfo->mName, 1, 0.0f, -1.0f, false);
    mParam->mStartFrame = mInfo->mStartFrame;
    mParam->mEndFrame = mInfo->mEndFrame;
}

void ActionSound::fn_8021FA40() {
    if (mInfo->fn_80220830()) mUnk10 = true;
}

void ActionSound::fn_8021FA80() {
    if (!mInfo->fn_80220830()) {
        mHandle = fn_8021F6F0(mSoundObject, mInfo->mSoundType, mInfo->mSoundId, -1, -1);
        mUnk10 = false;
    }
}

void ActionSound::fn_8021FAE0(u32 frames) {
    if (mInfo->fn_80220830()) {
        mUnk10 = false;
        return;
    }
    JAISound* sound = mHandle->mSound;
    s32 type = mInfo->mSoundType;
    if (sound && (type == 0 || type == 2 || type == 4 || type == 10) && mHandle) sound->stop(frames);
}

void ActionSound::fn_8021FB80() {
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

void ActionSound::fn_8021FC10(f32 rate) {
    if (mUnk10) {
        mHandle = fn_8021F7E0(mSoundObject, mInfo->mSoundType, mInfo->mSoundId, -1, -1, -1);
        fn_8021FD50(rate);
    }
}

bool ActionSound::play(s32 mode, s32 pitch, s32 velocity, s32 volume) {
    bool played = false;
    if (mInfo->fn_80220830()) {
        if (mode == 0 || mode == 2) {
            mHandle = fn_8021F7E0(mSoundObject, mInfo->mSoundType, mInfo->mSoundId, pitch, velocity, volume);
            played = true;
        }
    } else if (mode == 0 || mode == 1) {
        mHandle = fn_8021F6F0(mSoundObject, mInfo->mSoundType, mInfo->mSoundId, pitch, velocity);
        played = true;
    }
    fn_8021FF00(pitch, velocity);
    return played;
}

void ActionSound::fn_8021FD50(f32 rate) {
    if (mHandle) {
        if (mInfo->mPitchType == 0) {
            f32 scale = fn_8021F910(0);
            f32 offset = fn_8021F910(1);
            f32 min = fn_8021F910(2);
            f32 max = fn_8021F910(3);
            f32 pitch = 1.0f + (offset + (rate - 1.0f) * scale);
            fn_8021F8C0(&pitch, min, max);
            MR::clampBoth(&pitch, 0.01f, 10.0f);
            mHandle->mSound->mParams.mMove.mParams.mPitch *= pitch;
        }
        if (mInfo->mVolumeType == 0) {
            f32 scale = fn_8021F930(0);
            f32 offset = fn_8021F930(1);
            f32 min = fn_8021F930(2);
            f32 max = fn_8021F930(3);
            f32 volume = 1.0f + (offset + (rate - 1.0f) * scale);
            fn_8021F8C0(&volume, min, max);
            MR::clampBoth(&volume, 0.0f, 2.0f);
            mHandle->mSound->mParams.mMove.mParams.mVolume *= volume;
        }
    }
}

// NONMATCHING: optimizer-disabled stack-base register and epilogue scheduling differ; branch graph and arithmetic match.
#pragma global_optimizer off
void ActionSound::fn_8021FF00(s32 pitchArg, s32 volumeArg) {
    if (mHandle && mHandle->mSound) {
        f32 pitch = 0.0f;
        s32 pitchType = mInfo->mPitchType;
        if (pitchType == -1) goto volume;
        if (pitchType == 1) goto pitch1;
        if (pitchType == 2) goto pitch2;
        goto volume;
    pitch1: {
        f32 scale = fn_8021F910(0);
        f32 offset = fn_8021F910(1);
        f32 min = fn_8021F910(2);
        f32 max = fn_8021F910(3);
        pitch = offset + pitchArg * scale;
        fn_8021F8C0(&pitch, min, max);
        MR::clampBoth(&pitch, 0.01f, 10.0f);
        mHandle->mSound->mParams.mMove.mParams.mPitch *= pitch;
        goto volume;
    }
    pitch2: {
        f32 scale = fn_8021F910(0);
        f32 offset = fn_8021F910(1);
        f32 min = fn_8021F910(2);
        f32 max = fn_8021F910(3);
        pitch = offset + volumeArg * scale;
        fn_8021F8C0(&pitch, min, max);
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
        f32 scale = fn_8021F930(0);
        f32 offset = fn_8021F930(1);
        f32 min = fn_8021F930(2);
        f32 max = fn_8021F930(3);
        volume = offset + pitchArg * scale;
        fn_8021F8C0(&volume, min, max);
        MR::clampBoth(&volume, 0.0f, 2.0f);
        mHandle->mSound->mParams.mMove.mParams.mVolume *= volume;
        goto done;
    }
    volume2: {
        f32 scale = fn_8021F930(0);
        f32 offset = fn_8021F930(1);
        f32 min = fn_8021F930(2);
        f32 max = fn_8021F930(3);
        volume = offset + volumeArg * scale;
        fn_8021F8C0(&volume, min, max);
        MR::clampBoth(&volume, 0.0f, 2.0f);
        mHandle->mSound->mParams.mMove.mParams.mVolume *= volume;
    }
    }
done:
    return;
}
#pragma global_optimizer on
