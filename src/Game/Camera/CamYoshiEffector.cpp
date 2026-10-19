#define MR_TAN_DEGREE_NOINLINE
#include "Game/Camera/CamYoshiEffector.hpp"
#include "Game/Camera/CameraLocalUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/YoshiUtil.hpp"

CamYoshiEffector::CamYoshiEffector() {
}

// NONMATCHING: vector temporary registers and stack placement.
void CamYoshiEffector::update(CameraMan* camera) {
    if (!MR::isPlayerElementModeYoshiDash()) {
        return;
    }

    const TVec3f& watchPos = CameraLocalUtil::getWatchPos(camera);
    TVec3f direction(CameraLocalUtil::getPos(camera));
    direction -= watchPos;
    f32 distance = direction.length();
    if (MR::normalizeOrZero(&direction)) {
        return;
    }

    f32 dashHalfFovyTan = MR::tanDegree(32.5f);
    f32 currentHalfFovyTan = MR::tanDegree(0.5f * CameraLocalUtil::getFovy(camera));
    distance = distance * currentHalfFovyTan / dashHalfFovyTan;
    TVec3f offset = direction * distance;
    CameraLocalUtil::setPos(camera, CameraLocalUtil::getWatchPos(camera) + offset);
    CameraLocalUtil::setFovy(camera, 65.0f);
}
