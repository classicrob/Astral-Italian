#pragma once
#include <revolution.h>
struct ResourceCsvGroupInfo {
    const char* mName;
    s32 mCount;
    s32 mStartIndex;
    u32 _C;
    u32 _10;
};
class ResourceCsvGroupHolder {
public:
    const ResourceCsvGroupInfo* fn_804CC650(const char*) const;
    s32 mCount;
    u32 _4;
    ResourceCsvGroupInfo* mGroups;
};
