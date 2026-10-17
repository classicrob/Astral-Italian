#pragma once

#include <revolution/types.h>

class JAISoundHandle;

class AudTamakoroBgmCtrl {
public:
    AudTamakoroBgmCtrl();

    void init();
    void init(const char*);  // SMG2 signature (target init__18AudTamakoroBgmCtrlFPCc)
    void control(f32, bool, s32);
    void modifyTempo(JAISoundHandle*, f32);
    void modifyPitch(JAISoundHandle*, f32);

    /* 0x00 */ u32 mSteps;
};
