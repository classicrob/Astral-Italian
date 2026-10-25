#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include <JSystem/JGeometry.hpp>

typedef void (*PosAttrFunc)(f32, f32, f32);
typedef void (*TexAttrFunc)(f32, f32);

class ElectricRailMovingPoint : public LiveActor {
public:
    ElectricRailMovingPoint(const char* pName = "移動電撃レール点");

    virtual ~ElectricRailMovingPoint() {}
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);

    /* 0x90 */ bool mHasShadow;
    /* 0x91 */ bool mIsActive;
};

class ElectricRailMoving : public LiveActor {
public:
    ElectricRailMoving(const char*);

    virtual ~ElectricRailMoving();
    virtual void init(const JMapInfoIter&);
    virtual void draw() const;
    virtual void attackSensor(HitSensor*, HitSensor*);

    void disappear();
    bool isTouchRail(const HitSensor*, TVec3f*, TVec3f*) const;
    void initMapToolInfo(const JMapInfoIter&);
    void initSensor();
    void initRail(const JMapInfoIter&);
    void initPoints();
    void initDisplayList();
    void drawRailGX(f32) const;
    void drawPlane(f32, f32, f32, f32) const;
    void drawPlaneGX(f32, f32, f32, f32) const;
    void setVertexAttribute(int, int, f32, f32, f32, f32, PosAttrFunc, TexAttrFunc) const;
    void updateHitSensorPos();
    bool updatePointPos();
    void updatePointPosAndModel();
    bool isValidCoord(f32) const;
    f32 getRepeatedCoord(f32) const;
    void calcPointPos(TVec3f*, f32) const;
    void move(f32);
    bool tryFreeze();
    void exeFreeze();
    void exeDisappear();
    void exeDisappeared();
    void exeWait() NO_INLINE;

    /* 0x90 */ s32 mSegmentNum;
    /* 0x94 */ f32 mMovementSpeed;
    /* 0x98 */ f32 mSegmentLength;
    /* 0x9C */ s32 mStackHeight;
    /* 0xA0 */ TPos3f mTexMtx;
    /* 0xD0 */ u8 mAlpha;
    u8 _CD;
    u8 _CE;
    u8 _CF;
    /* 0xD4 */ TVec3f* mSensorOffsets;
    /* 0xD8 */ ElectricRailMovingPoint* mMovingPoints;
    /* 0xDC */ s32 mMovingPointCount;
    /* 0xE0 */ void* mDisplayListBuffer;
    /* 0xE4 */ u32 mDisplayListSize;
    /* 0xE8 */ s32 mRailPosCount;
    /* 0xEC */ TVec3f* mRailPositions;
    /* 0xF0 */ f32 mScrollCoord;
    /* 0xF4 */ f32 mEaseIn;
};

namespace ElectricRailFunction {
    f32 getHipDropSensorRadius();
}
