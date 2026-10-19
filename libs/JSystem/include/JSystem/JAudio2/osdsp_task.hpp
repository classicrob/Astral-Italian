#pragma once

#ifdef GALAXY_NATIVE
#include "JSystem/JAudio2/dspproc.hpp"
#endif
#include <revolution/dsp.h>

extern DSPTaskInfo* DSP_prior_task;

#ifdef GALAXY_NATIVE
void DsyncFrame2(u32 param_0, JASDspAddr param_1, JASDspAddr param_2);
void DsyncFrame3(u32 param_0, JASDspAddr param_1, JASDspAddr param_2, JASDspAddr param_3, JASDspAddr param_4);
#else
void DsyncFrame2(u32 param_0, u32 param_1, u32 param_2);
#endif
