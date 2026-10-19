#include "Game/Camera/CamTranslatorMedianPlanet.hpp"
#include "Game/Camera/CameraParamChunk.hpp"

CamTranslatorMedianPlanet::CamTranslatorMedianPlanet(CameraMedianPlanet* pCamera) : mCamera(pCamera) {
}

void CamTranslatorMedianPlanet::setParam(const CameraParamChunk* pChunk) {
    CameraGeneralParam* general = pChunk->mGeneralParam;

    f32 rateDistMin = general->mWPoint.y;
    f32 rateDistMax = general->mWPoint.z;
    if (general->mNum1 == 0) {
        rateDistMin = 0.0f;
        rateDistMax = 0.0f;
    }

    mCamera->setParam(general->mString.getCharPtr(), general->mAxis.x, general->mAxis.y, general->mAngleA, general->mAxis.z, general->mAngleB,
                      general->mDist, general->mWPoint.x, rateDistMin, rateDistMax);
}

Camera* CamTranslatorMedianPlanet::getCamera() const {
    return mCamera;
}
