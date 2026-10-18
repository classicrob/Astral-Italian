#pragma once

#include "Game/Camera/Camera.hpp"

class CameraFreeze : public Camera {
public:
    CameraFreeze(const char* pName);

    virtual void reset();
    virtual CameraTargetObj* calc();
    virtual CamTranslatorBase* createTranslator();
    virtual bool isCorrectingErpPositionOff() const;
    virtual bool isSubjectiveCameraOff() const;
};
