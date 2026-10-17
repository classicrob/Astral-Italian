#pragma once

#include <revolution.h>

class ScenarioData;

class GalaxyStatusAccessor {
public:
    GalaxyStatusAccessor(const ScenarioData*);

    const char* getName() const;
    s32 getPowerStarNum() const;

    s32 getWorldNo() const;

    const ScenarioData* mScenarioData;  // 0x00
};

// Merged from Petari (core lane): declarations missing here


namespace MR {
    GalaxyStatusAccessor makeGalaxyStatusAccessor(const char*);
    GalaxyStatusAccessor makeCurrentGalaxyStatusAccessor();
};  // namespace MR
