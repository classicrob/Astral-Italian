#pragma once

#include <revolution.h>

class ScenarioData;

class GalaxyStatusAccessor {
public:
    s32 getZoneNum() const;
    s32 getZoneId(const char*) const;
    bool isHiddenStar(s32) const;
    GalaxyStatusAccessor(const ScenarioData*);

    const char* getName() const;
    s32 getPowerStarNum() const;

    s32 getWorldNo() const;

    // SMG2 additions (symbols.txt 0x804D1900..); only what the Screen units use
    s32 getNormalPowerStarNumWithoutComet() const;
    s32 getGreenStarNum() const;
    bool isValidComet(s32) const;
    bool isNotNormalStar(s32) const;
    bool isOpenScenario(s32) const;

    const ScenarioData* mScenarioData;  // 0x00
};

// Merged from Petari (core lane): declarations missing here


namespace MR {
    GalaxyStatusAccessor makeGalaxyStatusAccessor(const char*);
    GalaxyStatusAccessor makeCurrentGalaxyStatusAccessor();
};  // namespace MR
