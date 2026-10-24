#include "Game/Camera/CamTranslatorSpiral.hpp"
#include "Game/Camera/CameraParamChunk.hpp"

// The Wii reads the packed times as big-endian halfwords; native keeps numeric extraction.
void CamTranslatorSpiral::setParam(const CameraParamChunk* pChunk) {
    CameraGeneralParam* general = pChunk->mGeneralParam;

#ifdef GALAXY_NATIVE
    s32 endTime = general->getNum1HighS16();
    s32 startTime = general->getNum1LowS16();
#else
    const s16* times = reinterpret_cast< const s16* >(&general->mNum1);
    s32 endTime = times[0];
    s32 startTime = times[1];
#endif

    mCamera->setParam(general->mNum2, endTime, startTime, general->mWPoint.y, general->mAxis.y, general->mWPoint.z, general->mAxis.z,
                      general->mWPoint.x, general->mAxis.x);
}

Camera* CamTranslatorSpiral::getCamera() const {
    return mCamera;
}
