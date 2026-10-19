#pragma once

#include "Game/Camera/Camera.hpp"
#include "Game/Camera/CameraPadOrbitCtrl.hpp"

class CameraCubePlanet : public Camera {
public:
    CameraCubePlanet(const char* pName = "キューブ惑星カメラ");

    virtual void reset();
    virtual CameraTargetObj* calc();
    virtual CamTranslatorBase* createTranslator();
    virtual bool isEnableToReset() const;
    virtual bool isEnableToRoundRight() const;
    virtual bool isEnableToRoundLeft() const;

    void setParam(f32 dist, f32 angleX, f32 angleY) {
        mDist = dist;
        mAngleX = angleX;
        mAngleY = angleY;
    }

    /* 0x54 */ f32 mDist;
    /* 0x58 */ f32 mAngleX;
    /* 0x5C */ f32 mAngleY;
    /* 0x60 */ bool _60;
    /* 0x64 */ TVec3f mUp;
    /* 0x70 */ CameraPadOrbitCtrl* _70;
};
