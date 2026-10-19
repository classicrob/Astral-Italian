#pragma once

#include "Game/MapObj/MapPartsFunction.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class SpringValue;

class SpringStep {
public:
    SpringStep(f32, f32, f32);

    void addSpringVelocity(f32);
    void setSpringBaseValue(f32);

    /* 0x00 */ f32 mCondition;
    /* 0x04 */ f32 mSpeed;
    /* 0x08 */ f32 mAngle;
    /* 0x0C */ SpringValue* mSpringValue;
};

class MapPartsFloatingForce : public MapPartsFunction {
public:
    MapPartsFloatingForce(LiveActor* pActor);

    virtual ~MapPartsFloatingForce();
    virtual void init(const JMapInfoIter& rIter);
    virtual void start();

    void setObjectName(const char* pName);
    void initForNormalMotion(const JMapInfoIter& rIter);
    void initForSpringMotion(const JMapInfoIter& rIter);
    void updateVelocity();
    void updateVelocitySpring();
    bool tryOn();
    bool tryReturn();
    void exeWait();
    void exeMove();
    void exeMoveSpring();
    void exeMoveReturn();
    void exeMoveEnd();

    /* 0x28 */ SpringStep* mSpringStep;
    /* 0x2C */ const char* mObjectName;
    /* 0x30 */ f32 mMaxOffset;
    /* 0x34 */ f32 mRotateAngle;
    /* 0x38 */ f32 mRotateSpeed;
    /* 0x3C */ f32 mReturnMargin;
    /* 0x40 */ s32 mRotateAccelType;
    /* 0x44 */ TVec3f mBasePos;
    /* 0x50 */ TVec3f mUpDirection;
    /* 0x5C */ f32 mCurOffset;
    /* 0x60 */ f32 mVerticalSpeed;
    /* 0x64 */ TVec3f mMoveVelocity;
    /* 0x70 */ f32 mSpringKick;
};
