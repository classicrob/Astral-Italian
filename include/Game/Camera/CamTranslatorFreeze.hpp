#pragma once

#include "Game/Camera/CameraFreeze.hpp"

class CamTranslatorFreeze : public CamTranslatorBase {
public:
    CamTranslatorFreeze(CameraFreeze* pCamera) : mCamera(pCamera) {
    }

    virtual void setParam(const CameraParamChunk*);
    virtual Camera* getCamera() const;

    /* 0x4 */ CameraFreeze* mCamera;
};
