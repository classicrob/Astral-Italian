#include "Game/System/GameDataConst.hpp"
#include "Game/System/GameDataFunction.hpp"
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
