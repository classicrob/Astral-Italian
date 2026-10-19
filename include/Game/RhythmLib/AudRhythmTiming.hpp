#pragma once
#include <revolution/types.h>
namespace AudRhythmWrap {
    bool isBeatTiming(s32 beatPeriod, f32 offset);
    f32 wrapBeatPosition(s32 n, f32 t);
    u32 getBeatCount();
    u32 getPrevBeatCount();
    f32 getBeatPhase();
    f32 getPrevBeatPhase();
}
