#pragma once
#include <revolution/types.h>
struct AudBgmParam {
    AudBgmParam();
    void reset() NO_INLINE;
    u32 mSoundId;
    s32 mStartType, mStartFrame;
    bool mIsPrepare;
};
struct AudBgmParamGroup {
    AudBgmParamGroup();
    void reset() NO_INLINE;
    AudBgmParam* getParam(s32);
    u32 getSoundId(s32, s32);
    u32 getScenarioSoundId(s32);
    AudBgmParam* mParams;
    const char* mStageName;
    u32 mChangeSoundIds[5];
    s32 mChangeStates[5];
};
class AudBgmParamTable {
public:
    AudBgmParamTable();
    void reset() NO_INLINE;
    void load();
    AudBgmParamGroup* findGroup(const char*) NO_INLINE;
    AudBgmParam* getParam(const char*, s32);
    u32 getSoundId(const char*, s32, s32);
    s32 getChangeState(const char*, s32);
    u32 getScenarioSoundId(const char*, s32);
    AudBgmParamGroup* mGroups;
    s32 mGroupCount;
};
