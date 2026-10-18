#pragma once

#include <revolution/types.h>

struct AudBgmParamGroup {
    AudBgmParamGroup();  // fn_800899D0

    /* 0x00 */ void* mParams;  // 11 entries of 0x10 bytes
};

// BgmParam.bcsv table (/AudioRes/Info/BgmParam.arc)
class AudBgmParamTable {
public:
    AudBgmParamTable();  // fn_80089B60

    /* 0x00 */ AudBgmParamGroup* mGroups;  // 80 groups of 0x30 bytes
    /* 0x04 */ s32 _4;
};
