#include "Game/System/GalaxyDataTableAccessor.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceDirector.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemDataHolder.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"

namespace {
    GalaxyDataTableAccessor* getGalaxyDataTableAccessor() NO_INLINE {
        return SingletonHolder< GameSystem >::get()->mSequenceDirector->mGalaxyDataTableAccessor;
    }
};  // namespace

namespace GameDataFunction {
    const char* getWorldFinalGalaxyName(s32 worldId) {
        return getGalaxyDataTableAccessor()->getGalaxyNameByValue("GrandStarNo", worldId);
    }

    s32 getGrandStarNo(const char* pGalaxyName) {
        return getGalaxyDataTableAccessor()->getValueS32(pGalaxyName, "GrandStarNo");
    }

    s32 getCometAppearMedalNum(const char* pGalaxyName) {
        return getGalaxyDataTableAccessor()->getValueS32(pGalaxyName, "CometAppearMedalNum");
    }
};  // namespace GameDataFunction

GalaxyDataTableAccessor::GalaxyDataTableAccessor(GameSystemDataHolder* pDataHolder) {
    mDataHolder = pDataHolder;
}

s32 GalaxyDataTableAccessor::getValueS32(const char* pGalaxyName, const char* pKey) const {
    s32 value;
    MR::getCsvDataS32ByElement(&value, mDataHolder->mGalaxyDataTable, "GalaxyName", pGalaxyName, pKey);
    return value;
}

const char* GalaxyDataTableAccessor::getGalaxyNameByValue(const char* pKey, s32 data) const {
    const char* pName;
    MR::getCsvDataStrByElement(&pName, mDataHolder->mGalaxyDataTable, pKey, data, "GalaxyName");
    return pName;
}
