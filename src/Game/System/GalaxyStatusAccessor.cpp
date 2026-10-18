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


s32 GalaxyStatusAccessor::getPowerStarNum() const {
    return mScenarioData->getPowerStarNum();
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

const char* GalaxyStatusAccessor::getCometName(s32 scenarioNo) const {
    const char* pCometName = nullptr;

    if (mScenarioData->getValueString("Comet", scenarioNo, &pCometName)) {
        return pCometName;
    } else {
        return nullptr;
    }
}








bool GalaxyStatusAccessor::isOpened() const {
    if (mScenarioData->getPowerStarNum() == 0) {
        return true;
    }

    return GameDataFunction::isOnGameEventFlag(mScenarioData->mGalaxyName);
}

bool GalaxyStatusAccessor::hasPowerStar(s32 scenarioNo) const {
    if (mScenarioData->getPowerStarNum() == 0) {
        return false;
    }

    return GameDataFunction::hasPowerStar(mScenarioData->mGalaxyName, scenarioNo);
}


s32 GalaxyStatusAccessor::getPowerStarNumOwned() const {
    if (mScenarioData->getPowerStarNum() == 0) {
        return 0;
    }

    return GameDataFunction::getPowerStarNumOwned(mScenarioData->mGalaxyName);
}

namespace MR {
    GalaxyStatusAccessor makeGalaxyStatusAccessor(const char* pGalaxyName) {
        return ScenarioDataFunction::getScenarioDataParser()->makeAccessor(pGalaxyName);
    }

    GalaxyStatusAccessor makeCurrentGalaxyStatusAccessor() {
        return ScenarioDataFunction::getScenarioDataParser()->makeAccessor(MR::getCurrentStageName());
    }
};  // namespace MR


const char* GalaxyStatusAccessor::getAppearPowerStarObjName(s32 scenarioNo) const {
    const char* pObjName;
    bool ret = mScenarioData->getValueString("AppearPowerStarObj", scenarioNo, &pObjName);

    if (ret) {
        return pObjName;
    }

    return 0;
}


// Note: this function's existence is implied by the leftover string in .data,
// This ultimately gets stripped, leaving no symbol, however the string data remains.
// Given the structure of the nearby functions, it is likely this function would
// be structured similarly.


bool GalaxyStatusAccessor::isHiddenStar(s32 scenarioNo) const {
    return false;
}
