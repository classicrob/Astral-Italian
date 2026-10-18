#pragma once

#include "Game/Camera/Camera.hpp"

class CameraSphereTrundle;

class CamTranslatorSphereTrundle : public CamTranslatorBase {
public:
    CamTranslatorSphereTrundle(CameraSphereTrundle* pCamera) : mCamera(pCamera) {
    }

    virtual void setParam(const CameraParamChunk*);
    virtual Camera* getCamera() const;

    /* 0x4 */ CameraSphereTrundle* mCamera;
};

class CameraSphereTrundle : public Camera {
public:
    CameraSphereTrundle(const char* pName = "球トランドルカメラ");
    virtual ~CameraSphereTrundle();

    virtual void reset();
    virtual CameraTargetObj* calc();
    virtual CamTranslatorBase* createTranslator();

    /* 0x54 */ TVec3f mOffset;
    /* 0x60 */ TVec3f mLocalPos;
    /* 0x6C */ f32 mDist;
    /* 0x70 */ f32 mAngle;
    /* 0x74 */ TVec3f mFront;
    /* 0x80 */ TVec3f mSide;
    /* 0x8C */ TVec3f mUp;
};
