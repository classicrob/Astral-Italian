#pragma once

#include <revolution.h>

/// @brief Small counter object of the AudSeStrategy unit used for the low-life alert sound (0x80085D30 / 0x80085D40).
/// Original name unknown; layout from Mario::fn_8038C540.
class AudLifeAlertCounter {
public:
    void fn_80085D30(s32);  // _4 += arg
    void fn_80085D40();     // clear

    /* 0x00 */ s32 _0;
    /* 0x04 */ s32 _4;
    /* 0x08 */ bool _8;
    /* 0x09 */ bool _9;
};
