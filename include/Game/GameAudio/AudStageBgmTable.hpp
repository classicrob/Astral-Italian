#pragma once

#include <revolution.h>

namespace AudStageBgmTable {
    bool isStartOnWelcome(const char*, s32);
    s32 getWelcomeStartOffset(const char*, s32);
    bool isFirstVisitStart(const char*, s32);
    u32 getBgmId(const char*, s32);

    u32 getBgmState(const char*, s32);
};  // namespace AudStageBgmTable
