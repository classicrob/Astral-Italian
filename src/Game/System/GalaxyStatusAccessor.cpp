#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/Util.hpp"
#include "Game/System/GameDataConst.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/StringUtil.hpp"

GalaxyStatusAccessor::GalaxyStatusAccessor(const ScenarioData* pScenarioData) : mScenarioData(pScenarioData) {
}

const char* GalaxyStatusAccessor::getName() const {
    return mScenarioData->mGalaxyName;
}

s32 GalaxyStatusAccessor::getNormalPowerStarNumWithoutComet() const {
    s32 count = 0;
    for (s32 i = 0; i < mScenarioData->getNormalPowerStarNum(); i++) {
        if (getCometName(i + 1) == nullptr) {
            count++;
        }
    }
    return count;
}

s32 GalaxyStatusAccessor::getPowerStarNum() const {
    return mScenarioData->getPowerStarNum();
}

s32 GalaxyStatusAccessor::getGreenStarNum() const {
    s32 count = 0;
    for (s32 i = 1; i <= mScenarioData->getPowerStarNum(); i++) {
        if (mScenarioData->isPowerStarTypeGreen(i)) {
            count++;
        }
    }
    return count;
}

s32 GalaxyStatusAccessor::getZoneNum() const {
    return mScenarioData->getZoneNum();
}

s32 GalaxyStatusAccessor::getZoneId(const char* pZone) const {
    return mScenarioData->getZoneId(pZone);
}

const char* GalaxyStatusAccessor::getZoneName(int zoneId) const {
    return mScenarioData->getZoneName(zoneId);
}

const char* GalaxyStatusAccessor::getAppearPowerStarObjName(s32 scenario) const {
    const char* name;
    bool found = mScenarioData->getValueString("AppearPowerStarObj", scenario, &name);
    const char* comet = getCometName(scenario);
    bool purple = false;
    if (comet != nullptr && MR::isEqualString(comet, "Purple")) {
        purple = true;
    }
    if (purple && !MR::isStagePurpleCoinsMoreThan100(mScenarioData->mGalaxyName)) {
        return "１００枚コイン";
    }
    if (found) {
        return name;
    }
    return nullptr;
}

bool GalaxyStatusAccessor::hasCometScenario() const {
    for (s32 i = 1; i <= mScenarioData->getNormalPowerStarNum(); i++) {
        if (getCometName(i) != nullptr) {
            return true;
        }
    }
    return false;
}

s32 GalaxyStatusAccessor::getCometScenarioId() const {
    for (s32 i = 1; i <= mScenarioData->getNormalPowerStarNum(); i++) {
        if (getCometName(i) != nullptr) {
            return i;
        }
    }
    return 1;
}

bool GalaxyStatusAccessor::isValidComet(s32 scenario) const {
    return getCometName(scenario) != nullptr;
}

const char* GalaxyStatusAccessor::getCometName(s32 scenarioNo) const {
    const char* pCometName = nullptr;
    if (mScenarioData->getValueString("Comet", scenarioNo, &pCometName)) {
        return pCometName;
    } else {
        return nullptr;
    }
}

// NONMATCHING: regalloc; equivalent boolean temporary uses a preserved register (FP order/grouping checked: no FP instructions).
s32 GalaxyStatusAccessor::getCometLimitTimer(s32 scenario) const {
    s32 timer;
    bool valid = mScenarioData->getValueU32("CometLimitTimer", scenario, &timer);
    if (valid) {
        valid = false;
        if (timer != 0) valid = true;
    }
    return valid ? timer : 0;
}

bool GalaxyStatusAccessor::isHiddenStar(s32 scenario) const {
    return mScenarioData->isPowerStarTypeHidden(scenario);
}

bool GalaxyStatusAccessor::isGreenStar(s32 scenario) const {
    return mScenarioData->isPowerStarTypeGreen(scenario);
}

bool GalaxyStatusAccessor::isNotNormalStar(s32 scenario) const {
    bool result = true;
    if (mScenarioData->isPowerStarTypeHidden(scenario) == 0) {
        if (mScenarioData->isPowerStarTypeGreen(scenario) == 0) {
            result = false;
        }
    }
    return result;
}

bool GalaxyStatusAccessor::isGrandStar(s32 scenario) const {
    return GameDataConst::isGrandStar(mScenarioData->mGalaxyName, scenario);
}

bool GalaxyStatusAccessor::hasPowerStar(s32 scenarioNo) const {
    if (mScenarioData->getPowerStarNum() == 0) {
        return false;
    }
    return GameDataFunction::hasPowerStar(mScenarioData->mGalaxyName, scenarioNo);
}

bool GalaxyStatusAccessor::isOwnBronzeStar(s32 scenario) const {
    return GameDataFunction::hasBronzeStar(mScenarioData->mGalaxyName, scenario);
}

bool GalaxyStatusAccessor::isCompletedNoGreen() const {
    if (mScenarioData->getPowerStarNum() == 0) {
        return false;
    }
    if (!GameDataFunction::isOnGalaxyFlagTicoCoin(mScenarioData->mGalaxyName)) {
        return false;
    }
    s32 total = mScenarioData->getPowerStarNum();
    return static_cast<u32>(total) - static_cast<u32>(getPowerStarNumOwned()) == 0;
}

bool GalaxyStatusAccessor::isCompletedWithGreen() const {
    if (mScenarioData->getPowerStarNum() == 0) {
        return false;
    }
    if (!GameDataFunction::isOnGalaxyFlagTicoCoin(mScenarioData->mGalaxyName)) {
        return false;
    }
    s32 total = mScenarioData->getPowerStarNum();
    s32 owned = getPowerStarNumOwned();
    s32 ownedNormal = owned - getGreenStarNumOwned();
    return ownedNormal >= total - getGreenStarNum();
}

bool GalaxyStatusAccessor::isCompletedGreenStars() const {
    s32 total = getGreenStarNum();
    return static_cast<u32>(total) - static_cast<u32>(getGreenStarNumOwned()) == 0;
}

s32 GalaxyStatusAccessor::getPowerStarNumOwnedTotal() const {
    if (mScenarioData->getPowerStarNum() == 0) {
        return 0;
    }
    return GameDataFunction::getPowerStarNumOwned(mScenarioData->mGalaxyName);
}

bool GalaxyStatusAccessor::isOpened() const {
    if (mScenarioData->getPowerStarNum() == 0) {
        return true;
    }
    return GameDataFunction::isGalaxyStateOpened(mScenarioData->mGalaxyName);
}

bool GalaxyStatusAccessor::isOpenScenario(s32 scenario) const {
    if (hasPowerStar(scenario)) {
        return true;
    }
    if (mScenarioData->isPowerStarTypeGreen(scenario)) {
        return GameDataFunction::isOnGameEventFlagEnding120Stars();
    }
    if (isValidComet(scenario)) {
        return MR::getGalaxyStarTotalNum(mScenarioData->mGalaxyName);
    }
    if (GameDataFunction::existsGameEventFlagOpenScenario(mScenarioData->mGalaxyName, scenario)) {
        return GameDataFunction::isOnGameEventFlagOpenScenario(mScenarioData->mGalaxyName, scenario);
    }
    if (MR::isStageDimensionBigCastle(mScenarioData->mGalaxyName)) {
        return GameDataFunction::isOnGameEventFlagFinalGalaxyAppearance();
    }
    return true;
}

bool GalaxyStatusAccessor::isExistPowerStarAtScenario(s32 scenario, s32 star) const {
    s32 mask;
    s32 value;
    if (mScenarioData->getValueU32("PowerStarId", scenario, &mask)) {
        value = mask;
    } else {
        value = 0;
    }
    return (value & (1 << (star - 1))) != 0;
}

s32 GalaxyStatusAccessor::getWorldNo() const {
    return mScenarioData->getWorldNo();
}

s32 GalaxyStatusAccessor::getPowerStarNumOwned() const {
    s32 count = 0;
    for (s32 i = 1; i <= mScenarioData->getPowerStarNum(); i++) {
        if (hasPowerStar(i) && !GameDataFunction::hasBronzeStar(mScenarioData->mGalaxyName, i)) {
            count++;
        }
    }
    return count;
}

s32 GalaxyStatusAccessor::getGreenStarNumOwned() const {
    s32 count = 0;
    for (s32 i = 1; i <= mScenarioData->getPowerStarNum(); i++) {
        if (mScenarioData->isPowerStarTypeGreen(i) && hasPowerStar(i)) {
            count++;
        }
    }
    return count;
}

namespace MR {
    GalaxyStatusAccessor makeGalaxyStatusAccessor(const char* pGalaxyName) {
        return ScenarioDataFunction::getScenarioDataParser()->makeAccessor(pGalaxyName);
    }

    GalaxyStatusAccessor makeCurrentGalaxyStatusAccessor() {
        return ScenarioDataFunction::getScenarioDataParser()->makeAccessor(MR::getCurrentStageName());
    }
}
