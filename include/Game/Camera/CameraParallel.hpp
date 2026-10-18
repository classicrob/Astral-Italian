#pragma once

#include "Game/Camera/Camera.hpp"

class CameraParallel : public Camera {
public:
    CameraParallel(const char* pName = "平行カメラ");

    virtual void reset();
    virtual CameraTargetObj* calc();
    virtual CamTranslatorBase* createTranslator();
    virtual bool isEnableToRoundLeft() const;
    virtual bool isEnableToRoundRight() const;
    virtual bool isEnableToReset() const;

    void calcIdealPose();
    void calcRound();

    void setParam(const TVec2f& angle, f32 dist, bool enabled) {
        mAngleX = angle.x;
        mAngleY = angle.y;
        mDist = dist;
        mIsEnableValid = enabled;
    }

    /* 0x54 */ f32 mAngleX;
    /* 0x58 */ f32 mAngleY;
    /* 0x5C */ f32 mDist;
    /* 0x60 */ bool mIsEnableValid;
    /* 0x61 */ bool mIsRounding;  // target keeps an 8-entry array here (0x61..0x68); only element 0 is modelled
    /* 0x62 */ bool _62[7];
    /* 0x6C */ f32 mRoundTarget;  // 8-entry array 0x6C..0x88 in target; only element 0 is modelled
    /* 0x70 */ f32 _70[7];
    /* 0x8C */ f32 mRoundAngle;   // 8-entry array 0x8C..0xA8 in target
    /* 0x90 */ f32 _90[7];
    /* 0xAC */ f32 mRoundAddition;  // 8-entry array 0xAC..0xC8 in target
    /* 0xB0 */ f32 _B0[7];
    /* 0xCC */ void* _CC;  // PLACEHOLDER(core): pointer zeroed in ctor before createVPanObj; resolve when its type is known
};
