#include "Game/Camera/CamHeliEffector.hpp"
#include "Game/Camera/CameraLocalUtil.hpp"
#include "Game/Camera/CameraTargetObj.hpp"
#include "Game/Player/MarioActor.hpp"  // DO NOT REMOVE
#include "Game/Util/MathUtil.hpp"

CamHeliEffector::CamHeliEffector() : mFrame(), mStrength() {
}

// NONMATCHING: vector temporary addresses and register allocation.
void CamHeliEffector::update(CameraMan* pCamera) {
    if (!CameraLocalUtil::getTarget(pCamera)->isCameraStateOn(1)) {
        mFrame = 0;
        if (mStrength == 0.0f) {
            return;
        }

        f32 val = mStrength;
        val *= 0.98f;
        mStrength = val;
        if (val < 0.001f) {
            mStrength = 0.0f;
        }
    } else {
        f32 val = mFrame / 60.0f;
        if (val > 1.0f) {
            val = 1.0f;
        }

        val *= val;
        val *= val;
        mStrength += (1.0f - mStrength) * val;
        mFrame++;
    }

    TVec3f up = CameraLocalUtil::getTarget(pCamera)->getUpVec();
    TVec3f diff = CameraLocalUtil::getPos(pCamera) - CameraLocalUtil::getWatchPos(pCamera);

    if (MR::isNearZero(diff)) {
        return;
    }

    TVec3f direction = diff;
    MR::normalize(&direction);

    f32 angle = direction.angle(up);
    if (angle < MR::pi() / 6.0f) {
        return;
    }

    TQuat4f quat;
    const f32 rate = (angle - MR::pi() / 6.0f) / angle;
    quat.setRotate(direction, up, rate * mStrength);
    quat.transform(diff);

    CameraLocalUtil::setPos(pCamera, diff + CameraLocalUtil::getWatchPos(pCamera));

    TVec3f newUp(CameraLocalUtil::getUpVec(pCamera));
    quat.transform(newUp);
    CameraLocalUtil::setUpVec(pCamera, newUp);
}
