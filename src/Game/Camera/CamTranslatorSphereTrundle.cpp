#include "Game/Camera/CameraParamChunk.hpp"
#include "Game/Camera/CameraSphereTrundle.hpp"
#include "Game/Util/MathUtil.hpp"

void CamTranslatorSphereTrundle::setParam(const CameraParamChunk* pChunk) {
    CameraGeneralParam* general = pChunk->mGeneralParam;

    TVec3f axis = general->mAxis;
    if (MR::isNearZero(axis, 0.001f)) {
        return;
    }
    MR::normalize(&axis);

    CameraSphereTrundle* camera = mCamera;
    f32 angle = general->mAngleA;
    f32 dist = general->mDist;
    camera->mOffset.set(general->mWPoint);
    camera->mLocalPos.set(axis);
    camera->mDist = dist;
    camera->mAngle = angle;
}

Camera* CamTranslatorSphereTrundle::getCamera() const {
    return mCamera;
}
