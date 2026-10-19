#pragma once
#include <revolution.h>
class ActionSoundInfo;
class JMapInfoIter;
struct ResourceCsvGroupInfo;
class ActionSoundData {
public:
    ActionSoundData(const char*, const ResourceCsvGroupInfo*);
    ActionSoundInfo* getInfo(s32) const;
    ActionSoundInfo* findInfoByName(const char*) const;
    const char* mName;
    ActionSoundInfo** mInfos;
    s32 mCount;
};
ActionSoundData* createActionSoundData(const char*);
const char* getArchivePath();
