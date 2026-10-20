#pragma once

#include <revolution/types.h>

class JASRegisterParam {
public:
#ifdef GALAXY_NATIVE
    enum RegID : u32 { REG_A, REG_B, REG_S, REG_F, REG_X, REG_Y };
#else
    enum RegID { REG_A, REG_B, REG_S, REG_F, REG_X, REG_Y };
#endif

    JASRegisterParam();

    void init();
    void write(JASRegisterParam::RegID, u32);
    u32 read(JASRegisterParam::RegID);

    /* 0x0 */ u16 _0[14];
};
