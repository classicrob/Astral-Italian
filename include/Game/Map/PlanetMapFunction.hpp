#pragma once

#include <revolution/types.h>

class LiveActor;

namespace PlanetMapFunction {
    void verifyEffects(LiveActor*, s32*, const char*, s32);
    bool tryEmitEffects(LiveActor*, const char*, s32);
    bool tryDeleteEffects(LiveActor*, const char*, s32);
    bool tryForceDeleteEffects(LiveActor*, const char*, s32);
}
