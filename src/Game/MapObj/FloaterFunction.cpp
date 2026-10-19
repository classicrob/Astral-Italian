#include "Game/MapObj/FloaterFunction.hpp"
#include "Game/Util/StringUtil.hpp"

namespace FloaterFunction {
    bool isExistImpactOnRide(const char* pObjName) {
        return MR::isEqualString("LavaHomeFloaterA", pObjName) || MR::isEqualString("LavaHomeFloaterB", pObjName) ||
               MR::isEqualString("LavaFloatingStepA", pObjName) || MR::isEqualString("LavaFloatingStepB", pObjName) ||
               MR::isEqualString("LavaFloatingStepC", pObjName) || MR::isEqualString("LavaFloatingStepD", pObjName);
    }

    bool isExistSpeedUpLanding(const char* pObjName) {
        return MR::isEqualString("FloaterOtakingFloaterA", pObjName);
    }
};  // namespace FloaterFunction
