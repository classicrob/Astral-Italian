#pragma once
#include <revolution.h>
class JMapInfoIter;
class ActionSoundData;
class ActionSoundInfo {
public:
    ActionSoundInfo(JMapInfoIter, const ActionSoundData*);
    void setDefault();
    void fn_802207A0();
    bool fn_80220830() const;
    const char* getSoundLabel();
    s32 fn_80220890() const;
    static bool fn_802208A0(s32);
    s32 mTriggerType;
    const char* mName;
    s32 mSoundType;
    const char* mSoundLabel;
    s32 mStartFrame;
    s32 mEndFrame;
    const char* mJointName;
    s32 mPitchType;
    f32 mPitchArgs[4];
    s32 mVolumeType;
    f32 mVolumeArgs[4];
    s32 mSoundId;
};
