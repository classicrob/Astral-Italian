#pragma once
#include <revolution.h>
class JMapInfo;
struct ResourceCsvGroupInfo {
    ResourceCsvGroupInfo();
    const char* mName;
    s32 mCount;
    s32 mStartIndex;
    s32 _C; // AutoEffectList row count
    u32 _10; // AutoEffectList first row; initially UINT_MAX
};
class ResourceCsvGroupHolder {
public:
    ResourceCsvGroupHolder();
    void reset();
    const ResourceCsvGroupInfo* findGroup(const char*) const;
    void initActionSoundGroups();
    void initAutoEffectGroups();
    s32 mCount;
    u32 _4; // capacity1280
    ResourceCsvGroupInfo* mGroups;
    JMapInfo* mActionSoundCsv;
    JMapInfo* mAutoEffectCsv;
};
const char* getEffectArchivePath();
