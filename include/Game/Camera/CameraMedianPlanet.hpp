#pragma once

#include "Game/Camera/Camera.hpp"
#include <JSystem/JGeometry.hpp>

class CameraMedianPlanet : public Camera {
public:
    CameraMedianPlanet(const char* pName = "中点注目プラネットカメラ");
    virtual ~CameraMedianPlanet();

    virtual void reset();
    virtual CameraTargetObj* calc();
    virtual CamTranslatorBase* createTranslator();

    void getPrevFront(TVec3f*);
    void constrainDipAngle(TVec3f*);
    void makeTargetMtx(TPos3f*);
    void peepMtx(TPos3f*);
    void rotate33(TPos3f*, const TVec3f&, f32);
    void constrainLength(TVec3f*);
    f32 getMaxRate();
    void getRegisterUpVec(TVec3f*);
    void calcWatchPos(TVec3f*);
    bool calcUpVec(TVec3f*, const TVec3f&, const TVec3f&);

    void setParam(const char* pName, f32 distMin, f32 distMax, f32 angleMin, f32 constrainRate, f32 peepAngle, f32 dist, f32 partialRate, f32 rateDistMin, f32 rateDistMax) {
        mString = pName;
        mDistMin = distMin;
        mDistMax = distMax;
        mAngleMin = angleMin;
        mPeepAngle = peepAngle;
        mDist = dist;
        mAngleConstrainRate = constrainRate;
        mPartialRate = partialRate;
        mRateDistMin = rateDistMin;
        mRateDistMax = rateDistMax;
    }

    /* 0x54 */ const char* mString;
    /* 0x58 */ f32 mDistMin;
    /* 0x5C */ f32 mDistMax;
    /* 0x60 */ f32 mAngleMin;
    /* 0x64 */ f32 mPeepAngle;
    /* 0x68 */ f32 mDist;
    /* 0x6C */ f32 mAngleConstrainRate;
    /* 0x70 */ f32 mPartialRate;
    /* 0x74 */ f32 mRateDistMin;
    /* 0x78 */ f32 mRateDistMax;
};
