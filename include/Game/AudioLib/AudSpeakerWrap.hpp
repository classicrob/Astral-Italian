#pragma once

#include <revolution/types.h>

namespace AudSpeakerWrap {
    const char* getResName();
    u32 getRequiredHeapSize();
    void start(s32, s32, s32 = 0, f32 = 1.0f);
    void startLevel(s32, s32);
    void stop(s32, s32);
    s32 findFreeSoundHandle() NO_INLINE;
    bool isPlayable(s32);
    // PLACEHOLDER(core): unnamed fn_80086880 (AudSpeakerWrap.cpp area, called by MR::startCSSoundAfterPlayed); resolve when named
    void fn_80086880(s32, s32);
};  // namespace AudSpeakerWrap
