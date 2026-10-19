#include "Game/System/GameDataConst.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSystemDataHolder.hpp"
#include "Game/Util.hpp"

bool GameDataConst::isGrandStar(const char* pGalaxyName, s32 starId) {
    if (starId != 1) {
        return false;
    }

    for (s32 i = 1; i <= 6; i++) {
        if (MR::isEqualString(pGalaxyName, GameDataFunction::getWorldFinalGalaxyName(i))) {
            return true;
        }
    }

    return false;
}

s32 GameDataConst::getWorldNo(const char* pGalaxyName) {
    const JMapInfo* list = GameSystemDataFunction::getGalaxyWorldOrderList();
    u32 worldNo;
    JMapInfoIter galaxy = MR::getCsvDataU32ByElement(&worldNo, list, "EnName", pGalaxyName, "WorldNo");
    const char* firstGalaxy;
    JMapInfoIter first = MR::getCsvDataStrByElement(&firstGalaxy, list, "WorldNo", worldNo, "EnName");
    return galaxy.mIndex - first.mIndex;
}
