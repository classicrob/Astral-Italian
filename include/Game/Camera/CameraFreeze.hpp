#pragma once

#include "Game/Camera/Camera.hpp"

class CameraFreeze : public Camera {
public:
    CameraFreeze(const char* pName = "CameraFreeze");

    virtual void reset();
    virtual CameraTargetObj* calc();
    virtual CamTranslatorBase* createTranslator();
    virtual bool isUnknownOff() const;
    virtual bool isCorrectingErpPositionOff() const;
};
