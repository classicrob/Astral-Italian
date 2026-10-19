#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/ShadowVolumeDrawer.hpp"

class ElectricRailPoint : public LiveActor {
public:
    ElectricRailPoint(const char* name = "電撃レール点");

    virtual ~ElectricRailPoint();
    virtual void init(const JMapInfoIter&);
    virtual void control();
    virtual void attackSensor(HitSensor*, HitSensor*);

    /* 0x90 */ bool mHasShadow;
    /* 0x91 */ bool mIsActive;
};

class ElectricRailSeparator {
public:
    ElectricRailSeparator();

    void setup(const TVec3f&, const TVec3f&, const TVec3f&);

    /* 0x00 */ TVec3f mPos;
    /* 0x0C */ TVec3f mSide;
    /* 0x18 */ TVec3f mUp;
    /* 0x24 */ TVec3f mFront;
    /* 0x30 */ TVec3f mGravity;
};

class ElectricRailShadowDrawer : public ShadowVolumeDrawer {
public:
    ElectricRailShadowDrawer(const LiveActor*, ElectricRailSeparator*, s32);

    virtual void loadModelDrawMtx() const;
    virtual void drawShape() const;
    virtual bool isDraw() const;

    void drawShadowVolumeShape(ElectricRailSeparator*, s32) const;

    /* 0x24 */ const LiveActor* mHost;
    /* 0x28 */ void* mDisplayListBuffer;
    /* 0x2C */ u32 mDisplayListBufferSize;
};

class ElectricRail : public LiveActor {
public:
    ElectricRail(const char*);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void draw() const;
    virtual void makeActorAppeared();
    virtual void makeActorDead();
    virtual void attackSensor(HitSensor*, HitSensor*);

    void disappear();
    void initMapToolInfo(const JMapInfoIter&);
    void initSensor();
    void initPoints();
    void initSeparators();
    void initDisplayList();
    void drawRailGX(f32) const;
    void drawPlane(f32, f32, f32, f32) const;
    void drawPlaneGX(f32, f32, f32, f32) const;
    void initShadow(const JMapInfoIter&);
    void updateHitSensorPos();
    void calcGravity(TVec3f*, const TVec3f&) const;

    void exeDisappear();
    void exeWait();
    void exeDisappeared();

    /* 0x90 */ ElectricRailPoint* mPoints;
    /* 0x94 */ s32 mPointCount;
    /* 0x98 */ TVec3f* mSensorPos;
    /* 0x9C */ ElectricRailSeparator* mSeparators;
    /* 0xA0 */ s32 mSeparatorCount;
    /* 0xA4 */ void* mDisplayListBuffer;
    /* 0xA8 */ u32 mDisplayListSize;
    /* 0xAC */ s32 mRailHeight;  // Obj_arg0
    /* 0xB0 */ f32 mEaseIn;
    /* 0xB4 */ ElectricRailShadowDrawer* mShadowDrawer;
    /* 0xB8 */ bool mIsCalcGravity;  // Obj_arg4
    /* 0xB9 */ bool mIsKoopaBeam;  // Obj_arg5
};

namespace ElectricRailFunction {
    f32 getHipDropSensorRadius();
}
