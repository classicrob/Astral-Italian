#pragma once
#include <revolution/types.h>
class AudStageBgmTable {
public:
    static bool isStartOnWelcome(const char*, s32);
    static s32 getWelcomeStartOffset(const char*, s32);
    static bool isFirstVisitStart(const char*, s32);
};
