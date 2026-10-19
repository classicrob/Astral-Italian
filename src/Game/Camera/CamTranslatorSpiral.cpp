#include "Game/Camera/CamTranslatorSpiral.hpp"
#include "Game/Camera/CameraParamChunk.hpp"

// NONMATCHING: explicit packed-value extraction replaces native-endian pointer aliasing.
void CamTranslatorSpiral::setParam(const CameraParamChunk* pChunk) {
    CameraGeneralParam* general = pChunk->mGeneralParam;

    s32 endTime = general->getNum1HighS16();
    s32 startTime = general->getNum1LowS16();

    mCamera->setParam(general->mNum2, endTime, startTime, general->mWPoint.y, general->mAxis.y, general->mWPoint.z, general->mAxis.z,
                      general->mWPoint.x, general->mAxis.x);
}

Camera* CamTranslatorSpiral::getCamera() const {
    return mCamera;
}
