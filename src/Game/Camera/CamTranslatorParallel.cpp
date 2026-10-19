#include "Game/Camera/CamTranslatorParallel.hpp"
#include "Game/Camera/CameraParamChunk.hpp"
#include "Game/Util/MathUtil.hpp"

namespace {
    extern const f32 sPi;
    extern const f32 sHalfTurnDegrees;
}


void CamTranslatorParallel::setParam(const CameraParamChunk* pChunk) {
    CameraGeneralParam* general = pChunk->mGeneralParam;

    mCamera->setParam(TVec2f(sHalfTurnDegrees * general->mAngleB / sPi, sHalfTurnDegrees * general->mAngleA / sPi), general->mDist, general->mNum1 == 1, general->mNum2);
}

Camera* CamTranslatorParallel::getCamera() const {
    return mCamera;
}

namespace {
    const f32 sPi = 3.1415927f;
    const f32 sHalfTurnDegrees = 180.0f;
}
