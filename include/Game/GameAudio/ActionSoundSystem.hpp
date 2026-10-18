#pragma once

#include <revolution.h>

class ActionSoundInfo;

// Per-actor sound table returned by MR::getActionSoundData(name). Layout from the
// ActionSoundKeeper ctor and fn_80220400: 0x00 vtable, 0x04 ActionSoundInfo** entries, 0x08 entry count.
class ActionSoundSystem {
public:
    ActionSoundSystem(const char* pName);
    virtual ~ActionSoundSystem();

    ActionSoundInfo* getInfo(s32 index) const;

    ActionSoundInfo** mInfos;
    s32 mCount;
};
