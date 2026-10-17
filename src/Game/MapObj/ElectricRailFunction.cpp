#include "Game/MapObj/ElectricRailHolder.hpp"
#include "Game/Scene/SceneObjHolder.hpp"

namespace {
    const f32 cHitSensorRadius = 30.0f;

    ElectricRailHolder* getHolder() {
        return MR::getSceneObj< ElectricRailHolder* >(SceneObj_ElectricRailHolder);
    }
};  // namespace

void ElectricRailFunction::registerRail(const LiveActor* pActor, ElectricRailType railType) {
    MR::createSceneObj(SceneObj_ElectricRailHolder);
    ::getHolder()->registerRail(pActor, railType);
}

bool ElectricRailFunction::isTouchRail(const HitSensor* pSensor, TVec3f* pVec1, TVec3f* pVec2) {
    if (!MR::isExistSceneObj(SceneObj_ElectricRailHolder)) {
        return false;
    }

    return ::getHolder()->isTouchRail(pSensor, pVec1, pVec2);
}

f32 ElectricRailFunction::getHitSensorRadius() {
    return ::cHitSensorRadius;
}

u32 ElectricRailFunction::calcDisplayListSize(u32 size, s32 count) {
    return (((size * count + 3) / 32) + 2) * 32;
}
