#pragma once

#include "revolution/types.h"

namespace MR {
    void addCometTimer(s32);
};

// Merged from Petari (core lane): declarations missing here
#include "Game/NameObj/NameObj.hpp"
#include <JSystem/JGeometry/TVec.hpp>
class CometEventKeeper;
class PowerStarEventKeeper;
class PowerStarHolder;
class StageStateKeeper;
class TimeAttackEventKeeper;

namespace MR {
    void declareEventPowerStar(const char*, s32, bool);
    void initEventSystemAfterPlacement();
    bool isSuccessEventPowerStar(const char*, s32);
    bool isGreenEventPowerStar(const char*, s32);
    bool isRedEventPowerStar(const char*, s32);
    bool isGrandEventPowerStar(const char*, s32);
    void appearEventPowerStar(const char*, s32, const TVec3f*, bool, bool);
    bool isEndEventPowerStarAppearDemo(const char*);
};  // namespace MR
