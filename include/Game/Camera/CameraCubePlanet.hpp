#pragma once

#include "Game/Camera/Camera.hpp"

class CameraCubePlanet : public Camera {
public:
    CameraCubePlanet(const char* pName = "キューブ惑星カメラ");

    virtual void reset();
    virtual CameraTargetObj* calc();
    virtual CamTranslatorBase* createTranslator();

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
    /* 0x70 */ void* _70;  // PLACEHOLDER(core): 0xC object {bool; f32} allocated in ctor via fn_80129070; resolve when its class is named
};
