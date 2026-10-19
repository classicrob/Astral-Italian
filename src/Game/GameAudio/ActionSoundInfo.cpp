#include <cstdio>

#include "Game/GameAudio/ActionSoundInfo.hpp"
#include "Game/Util/JMapInfo.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <JSystem/JAudio2/JAISound.hpp>

ActionSoundInfo::ActionSoundInfo(JMapInfoIter iter, const ActionSoundData*) {
    setDefault();
    mTriggerType = iter.getValueS32("TriggerType");
    mName = iter.getValueString("TriggerName");
    mSoundType = iter.getValueS32("SoundType");
    mSoundLabel = iter.getValueString("SoundLabel");
    mStartFrame = iter.getValueS32("StartFrame");
    mEndFrame = iter.getValueS32("EndFrame");
    mJointName = iter.getValueString("JointName");
    mPitchType = iter.getValueS32("PitchChangeType");
    mVolumeType = iter.getValueS32("VolumeChangeType");
    if (mJointName != nullptr && *mJointName == '\0') {
        mJointName = nullptr;
    }
    fn_802207A0();

    char buf[0x40];
    for (s32 i = 0; i < 4; i++) {
        snprintf(buf, 0x40, "PitchChangeArg%d", i + 1);
        mPitchArgs[i] = iter.getValueFloat(buf);
    }
    for (s32 i = 0; i < 4; i++) {
        snprintf(buf, 0x40, "VolumeChangeArg%d", i + 1);
        mVolumeArgs[i] = iter.getValueFloat(buf);
    }
}

void ActionSoundInfo::setDefault() {
    mTriggerType = -1;
    mName = nullptr;
    mSoundType = -1;
    mSoundLabel = nullptr;
    mStartFrame = 0;
    mEndFrame = -1;
    mJointName = nullptr;
    mPitchType = -1;
    mVolumeType = -1;
    mSoundId = -1;
    for (s32 i = 0; i < 4; i++) {
        mPitchArgs[i] = 0.0f;
    }
    for (s32 i = 0; i < 4; i++) {
        mVolumeArgs[i] = 0.0f;
    }
}

void ActionSoundInfo::fn_802207A0() {
    switch (mSoundType) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 10:
    case 11:
        mSoundId = MR::getSoundID(mSoundLabel);
        break;
    case 6:
    case 7:
        mSoundId = MR::getCSSoundID(mSoundLabel);
        break;
    case 8:
    case 9:
        mSoundId = MR::getMEID(mSoundLabel);
        break;
    }
}

bool ActionSoundInfo::fn_80220830() const {
    switch (mSoundType) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 9:
    case 11:
        return true;
    default:
        return false;
    }
}

const char* ActionSoundInfo::getSoundLabel() {
    return mSoundLabel;
}

s32 ActionSoundInfo::fn_80220890() const {
    return fn_802208A0(mSoundType);
}

bool ActionSoundInfo::fn_802208A0(s32 type) {
    switch (type) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 10:
    case 11:
        return true;
    case -1:
    case 6:
    case 7:
    case 8:
    case 9:
        return false;
    default:
        return false;
    }
}
