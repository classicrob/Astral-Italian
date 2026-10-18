#pragma once
#include <revolution.h>
class ActionSoundInfo;
class JMapInfoIter;
struct ResourceCsvGroupInfo;
class ActionSoundData {
public:
    ActionSoundData(const char*, const ResourceCsvGroupInfo*);
    ActionSoundInfo* getInfo(s32) const;
    ActionSoundInfo* fn_80220410(const char*) const;
    const char* mName;
    ActionSoundInfo** mInfos;
    s32 mCount;
};
ActionSoundData* fn_80220250(const char*);
const char* fn_80220490();
