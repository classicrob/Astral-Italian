#include "Game/Camera/CameraParamChunk.hpp"
#include "Game/Camera/CameraSphereTrundle.hpp"
#include "Game/Util/MathUtil.hpp"

void CamTranslatorSphereTrundle::setParam(const CameraParamChunk* pChunk) {
    CameraSphereTrundle* camera;
    CameraGeneralParam* general = pChunk->mGeneralParam;

    TVec3f axis = general->mAxis;
    if (MR::isNearZero(axis, 0.001f)) {
        return;
    }
    MR::normalize(&axis);

    camera = mCamera;
    f32 dist;
    f32 angle = general->mAngleA;
    dist = general->mDist;
    camera->mOffset.set(general->mWPoint);
    camera->mLocalPos.set(axis);
    camera->mDist = dist;
    camera->mAngle = angle;
}

Camera* CamTranslatorSphereTrundle::getCamera() const {
    return mCamera;
}
