#pragma once

#include <revolution.h>

class ScenarioData;

class GalaxyStatusAccessor {
public:
    GalaxyStatusAccessor() : mScenarioData(nullptr) {}
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
    // SMG2 additions (core lane, from symbols.txt): declared for the Petari port of GalaxyStatusAccessor.cpp
    const char* getZoneName(int) const;
    const char* getAppearPowerStarObjName(s32) const;
    bool hasCometScenario() const;
    s32 getCometScenarioId() const;
    const char* getCometName(s32) const;
    s32 getCometLimitTimer(s32) const;
    bool isGreenStar(s32) const;
    bool isGrandStar(s32) const;
    bool hasPowerStar(s32) const;
    bool isOwnBronzeStar(s32) const;
    bool isCompletedNoGreen() const;
    bool isCompletedWithGreen() const;
    bool isCompletedGreenStars() const;
    s32 getPowerStarNumOwnedTotal() const;
    bool isOpened() const;
    bool isExistPowerStarAtScenario(s32, s32) const;
    s32 getPowerStarNumOwned() const;
    s32 getGreenStarNumOwned() const;

    const ScenarioData* mScenarioData;  // 0x00
};

// Merged from Petari (core lane): declarations missing here


namespace MR {
    GalaxyStatusAccessor makeGalaxyStatusAccessor(const char*);
    GalaxyStatusAccessor makeCurrentGalaxyStatusAccessor();
};  // namespace MR
