#pragma once

#include <revolution/types.h>

class GalaxyMoveArgument;
class UserFile;

namespace GameSequenceFunction {
    void collectTicoCoinInStage();
    bool isCollectTicoCoinInStage();
    bool isCollectTicoCoinSaved();
    bool isPadModeSuperDreamer();
    bool tryNANDErrorSequence(s32);
    void requestChangeScenarioSelect(const char*);
    bool isAllowDreamerInStage();
    bool isPadMode2();
    void setClearedPowerStarNum(s32);
};
