#pragma once

#include "Game/Camera/Camera.hpp"

// PLACEHOLDER(core): descriptive name for the camera pad-orbit helper at80129070;
// resolve when the original class name is recovered.
class CameraPadOrbitCtrl {
public:
    CameraPadOrbitCtrl() NO_INLINE;
    void reset() NO_INLINE;
    void update(const TVec3f&, const TVec3f&, const TVec3f&, TVec3f*) NO_INLINE;
    void updateRotation(const TVec3f&, const TVec3f&, const TVec3f&, TVec3f*) NO_INLINE;
    void updateInput(const TVec3f&, const TVec3f&, const TVec3f&, TVec3f*) NO_INLINE;
    bool mRotating;
    f32 mAngle;
    f32 mTargetAngle;
};

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
